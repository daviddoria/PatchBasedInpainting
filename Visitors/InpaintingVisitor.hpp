/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef InpaintingVisitor_HPP
#define InpaintingVisitor_HPP

#include "Visitors/InpaintingVisitorParent.h"

// STL
#include <memory>

// Concepts
#include "Concepts/DescriptorVisitorConcept.hpp"

// Accept criteria
#include "ImageProcessing/BoundaryEnergy.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Submodules
#include <ITKHelpers/ITKHelpers.h>
#include <BoostHelpers/BoostHelpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>
#include <Utilities/Debug/Debug.h>

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It forwards
 * initialize_vertex and discover_vertex, the only two functions that need to know about
 * the descriptor type, to a visitor that models DescriptorVisitorConcept.
 * The visitor needs to know the patch size of the patch to be inpainted because it uses
 * this size to traverse the inpainted region to update the boundary.
 *
 * 'TImage' is provided a default value so that if 'image' is not passed to the constructor
 * the value is not required to be specified (because we don't have an image to operate on anyway).
 */
template <typename TGraph, typename TBoundaryNodeQueue,
          typename TDescriptorVisitor, typename TAcceptanceVisitor, typename TPriority>
class InpaintingVisitor : public InpaintingVisitorParent<TGraph>, public Debug
{
  BOOST_CONCEPT_ASSERT((DescriptorVisitorConcept<TDescriptorVisitor, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  /** The mask indicating the region to inpaint. */
  Mask* MaskImage;

  /** A queue to use to determine which patch to inpaint next. */
  std::shared_ptr<TBoundaryNodeQueue> BoundaryNodeQueue;

  /** A function to determine the priority of each target patch. */
  std::shared_ptr<TPriority> PriorityFunction;

  /** A visitor to do patch descriptor specific operations. */
  std::shared_ptr<TDescriptorVisitor> DescriptorVisitor;

  /** A visitor to perform specified actions when a match is accepted. */
  std::shared_ptr<TAcceptanceVisitor> AcceptanceVisitor;

  /** The radius of the patches to use to inpaint the image. */
  const unsigned int PatchHalfWidth;

  /** How many patches have been finished so far. */
  unsigned int NumberOfFinishedPatches = 0;

  /** As the image is inpainted, this flag determines if new source patches can be created from patches which are now valid. */
  bool AllowNewPatches = false;

  /** The full region (the mask and image should both be this size). */
  itk::ImageRegion<2> FullRegion;

  /** Track which nodes have been used as source nodes. */
  typedef std::set<itk::Index<2>, itk::Index<2>::LexicographicCompare> UsedNodesSetType;
  UsedNodesSetType UsedNodesSet;

  /** Track which pixels have been used as source pixels. */
  typedef itk::Image<bool, 2> CopiedPixelsImageType;
  CopiedPixelsImageType::Pointer CopiedPixelsImage;

  /** Track where pixels have been copied from. */
  typedef itk::Image<itk::Index<2>, 2> SourcePixelMapImageType;
  SourcePixelMapImageType::Pointer SourcePixelMapImage;

public:

  CopiedPixelsImageType* GetCopiedPixelsImage()
  {
    return this->CopiedPixelsImage;
  }

  SourcePixelMapImageType* GetSourcePixelMapImage()
  {
    return this->SourcePixelMapImage;
  }

  UsedNodesSetType* GetUsedNodesSetPointer()
  {
    return &this->UsedNodesSet;
  }

  void SetAllowNewPatches(const bool allowNewPatches)
  {
    this->AllowNewPatches = allowNewPatches;
  }

  /** Constructor. Everything must be specified in this constructor. (There is no default constructor). */
  InpaintingVisitor(Mask* const mask,
                    std::shared_ptr<TBoundaryNodeQueue> boundaryNodeQueue,
                    std::shared_ptr<TDescriptorVisitor> descriptorVisitor,
                    std::shared_ptr<TAcceptanceVisitor> acceptanceVisitor,
                    std::shared_ptr<TPriority> const priorityFunction,
                    const unsigned int patchHalfWidth,
                    const std::string& visitorName = "InpaintingVisitor") :
    InpaintingVisitorParent<TGraph>(visitorName),
    MaskImage(mask), BoundaryNodeQueue(boundaryNodeQueue), PriorityFunction(priorityFunction),
    DescriptorVisitor(descriptorVisitor), AcceptanceVisitor(acceptanceVisitor),
    PatchHalfWidth(patchHalfWidth)
  {
    this->FullRegion = this->MaskImage->GetLargestPossibleRegion();

    this->CopiedPixelsImage = CopiedPixelsImageType::New();
    this->CopiedPixelsImage->SetRegions(this->FullRegion);
    this->CopiedPixelsImage->Allocate();
    this->CopiedPixelsImage->FillBuffer(false);

    this->SourcePixelMapImage = SourcePixelMapImageType::New();
    this->SourcePixelMapImage->SetRegions(this->FullRegion);
    this->SourcePixelMapImage->Allocate();
    itk::Index<2> dummyIndex = {{-1, -1}};
    this->SourcePixelMapImage->FillBuffer(dummyIndex);

    // Initialize by setting all valid pixels to their own index
    itk::ImageRegionIteratorWithIndex<SourcePixelMapImageType>
        sourcePixelMapImageIterator(this->SourcePixelMapImage, this->FullRegion);
    while(!sourcePixelMapImageIterator.IsAtEnd())
    {
      if(this->MaskImage->IsValid(sourcePixelMapImageIterator.GetIndex()))
      {
        this->SourcePixelMapImage->SetPixel(sourcePixelMapImageIterator.GetIndex(),
                                            sourcePixelMapImageIterator.GetIndex());
      }

      ++sourcePixelMapImageIterator;
    }
  }

  void InitializeVertex(VertexDescriptorType v) const
  {
    this->DescriptorVisitor->InitializeVertex(v);
  }

  void DiscoverVertex(VertexDescriptorType v) const
  {
    this->DescriptorVisitor->DiscoverVertex(v);
  }

  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source)
  {
//    std::cout << "InpaintingVisitor::PotentialMatchMade: target " << target[0] << " " << target[1]
//              << " source: " << source[0] << " " << source[1] << std::endl;

    if(!this->MaskImage->IsValid(ITKHelpers::CreateIndex(source)))
    {
      std::stringstream ss;
      ss << "InpaintingVisitor::PotentialMatchMade: Potential source pixel " << source[0] << " " << source[1] << " is not valid in the mask!";
      throw std::runtime_error(ss.str());
    }

    if(!this->MaskImage->HasHoleNeighbor(ITKHelpers::CreateIndex(target)))
    {
      std::stringstream ss;
      ss << "InpaintingVisitor::PotentialMatchMade: Potential target pixel " << target[0] << " " << target[1]
         << " does not have a hole neighbor (which means it is not on the boundary)!";
      throw std::runtime_error(ss.str());
    }
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    float energy = 0.0f;
    return AcceptanceVisitor->AcceptMatch(target, source, energy);
  }

  /** The mask is inpainted with ValidValue in this function. */
  void FinishVertex(VertexDescriptorType targetNode, VertexDescriptorType sourceNode)
  {
    // Mark this pixel and the area around it as filled, and mark the mask in this region as filled.
    // Determine the new boundary, and setup the nodes in the boundary queue.

//    std::cout << "InpaintingVisitor::FinishVertex()" << std::endl;

    // Construct the region around the vertex
    itk::Index<2> indexToFinish = ITKHelpers::CreateIndex(targetNode);

    // Mark this node as having been used as a source node.
    this->UsedNodesSet.insert(indexToFinish);

    itk::ImageRegion<2> regionToFinishFull =
        ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, this->PatchHalfWidth);

    // Copy this region so that we can change (crop) the regionToFinish and still have a copy of the original region
    itk::ImageRegion<2> regionToFinish = regionToFinishFull;

    // Make sure the region is entirely inside the image
    // (because we allow target patches to not be entirely inside the image to handle the case where
    // the hole boundary is near the image boundary)
    regionToFinish.Crop(this->FullRegion);

    itk::Index<2> sourceRegionCenter = ITKHelpers::CreateIndex(sourceNode);
    itk::ImageRegion<2> sourceRegion =
        ITKHelpers::GetRegionInRadiusAroundPixel(sourceRegionCenter, this->PatchHalfWidth);

    sourceRegion = ITKHelpers::CropRegionAtPosition(sourceRegion, this->MaskImage->GetLargestPossibleRegion(),
                                                    regionToFinishFull);

    // Mark all pixels that were copied (in the hole region of the source patch) as having been used.
//    std::cout << "InpaintingVisitor::FinishVertex() mark pixels as used" << std::endl;
    itk::ImageRegionConstIteratorWithIndex<SourcePixelMapImageType> sourcePatchIterator(this->SourcePixelMapImage,
                                                                                        sourceRegion);
    itk::ImageRegionConstIteratorWithIndex<Mask> targetPatchIterator(this->MaskImage, regionToFinish);
    while(!sourcePatchIterator.IsAtEnd())
    {
      if(targetPatchIterator.Get() == this->MaskImage->GetHoleValue())
      {
        this->CopiedPixelsImage->SetPixel(sourcePatchIterator.GetIndex(), true); // Mark this pixel as used

        // Save the location from which this pixel came. We want to use the index value in the SourcePixelMapImage,
        // because the value might not equal the current index in the case where new patches are allowed.
        this->SourcePixelMapImage->SetPixel(targetPatchIterator.GetIndex(),
                                            sourcePatchIterator.Get());
      }

      ++sourcePatchIterator;
      ++targetPatchIterator;
    }

    if(this->DebugImages)
    {
      if(this->DebugLevel > 1)
      {
        ITKHelpers::WriteBoolImage(this->CopiedPixelsImage,
                                   Helpers::GetSequentialFileName("CopiedPixels",
                                                                  this->NumberOfFinishedPatches, "png", 3));
        ITKHelpers::WriteImage(this->MaskImage,
                               Helpers::GetSequentialFileName("Mask_Before",
                                                              this->NumberOfFinishedPatches, "png", 3));
      }
    }

    // Mark all the pixels in this region as filled in the mask.
//    std::cout << "InpaintingVisitor::FinishVertex() fill the mask" << std::endl;
    ITKHelpers::SetRegionToConstant(this->MaskImage, regionToFinish,
                                    this->MaskImage->GetValidValue());

    // Write an image of where the source and target patch were in this iteration.
//    if(this->DebugImages && this->Image)
//    {
//      typename TImage::PixelType red;
//      red.Fill(0);
//      red[0] = 255;

//      typename TImage::PixelType green;
//      green.Fill(0);
//      green[1] = 255;

//      typename TImage::Pointer patchesCopiedImage = TImage::New();
//      ITKHelpers::DeepCopy(this->Image, patchesCopiedImage.GetPointer());
//      ITKHelpers::OutlineRegion(patchesCopiedImage.GetPointer(), regionToFinish, red);

//      ITKHelpers::OutlineRegion(patchesCopiedImage.GetPointer(), sourceRegion, green);

//      ITKHelpers::WriteRGBImage(patchesCopiedImage.GetPointer(), Helpers::GetSequentialFileName("PatchesCopied", this->NumberOfFinishedPatches, "png", 3));

//      if(this->DebugLevel > 1)
//      {
//        ITKHelpers::WriteImage(this->MaskImage, Helpers::GetSequentialFileName("Mask_After", this->NumberOfFinishedPatches, "png", 3));
//      }
//    }

    // Update the priority function. This must be done AFTER the mask is filled,
    // as some of the Priority functors only compute things on the hole boundary, or only
    // use data from the valid region of the image (indicated by valid pixels in the mask).
//    std::cout << "InpaintingVisitor::FinishVertex() update priority" << std::endl;
    this->PriorityFunction->Update(sourceNode, targetNode, this->NumberOfFinishedPatches);
//    std::cout << "InpaintingVisitor::FinishVertex() finish update priority" << std::endl;
    // Initialize (if requested) all vertices in the newly filled region because they may now be valid source nodes.
    // (You may not want to do this in some cases (i.e. if the descriptors needed cannot be
    // computed on newly filled regions))

    if(this->AllowNewPatches)
    {
//      std::cout << "Initializing new vertices..." << std::endl;
      itk::ImageRegionConstIteratorWithIndex<Mask> gridIterator(this->MaskImage, regionToFinish);
      while(!gridIterator.IsAtEnd())
      {
        VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(gridIterator.GetIndex());

        InitializeVertex(v);
        ++gridIterator;
      }
    }

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
//    std::cout << "InpaintingVisitor::FinishVertex() iterator" << std::endl;
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(this->MaskImage, regionToFinish);
//    std::cout << "InpaintingVisitor::FinishVertex() finish iterator" << std::endl;

    typedef typename TBoundaryNodeQueue::HandleType HandleType;

    // Naive, unparallelizable way
//    while(!imageIterator.IsAtEnd())
//    {
//      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(imageIterator.GetIndex());

//      if(this->MaskImage->HasHoleNeighbor(imageIterator.GetIndex()))
//      {
//        float priority = this->PriorityFunction->ComputePriority(imageIterator.GetIndex());
//        this->BoundaryNodeQueue.push_or_update(v, priority);
//      }
//      else
//      {
//        this->BoundaryNodeQueue.mark_as_invalid(v);
//      }

//      ++imageIterator;
//    }

    // Parallelizable way - collect the pixels that need their priority computed
//    std::cout << "InpaintingVisitor::FinishVertex() Parallelizable way" << std::endl;
    typedef std::vector<itk::Index<2> > IndexVectorType;
    IndexVectorType pixelsToCompute;
    while(!imageIterator.IsAtEnd())
    {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(imageIterator.GetIndex());

      if(this->MaskImage->GetPixel(imageIterator.GetIndex()) == this->MaskImage->GetValidValue() &&
         this->MaskImage->HasHoleNeighbor(imageIterator.GetIndex()))
      {
        pixelsToCompute.push_back(imageIterator.GetIndex());
      }
      else
      {
        this->BoundaryNodeQueue->mark_as_invalid(v);
      }

      ++imageIterator;
    }

//    std::cout << "InpaintingVisitor::FinishVertex() update queue" << std::endl;
    #pragma omp parallel for
    for(IndexVectorType::const_iterator pixelIterator = pixelsToCompute.begin();
        pixelIterator < pixelsToCompute.end(); ++pixelIterator)
    {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(*pixelIterator);
      float priority = this->PriorityFunction->ComputePriority(*pixelIterator);
      #pragma omp critical // There are weird crashes without this guard
      this->BoundaryNodeQueue->push_or_update(v, priority);
    }

    // std::cout << "FinishVertex after traversing finishing region there are "
    //           << BoostHelpers::CountValidQueueNodes(BoundaryNodeQueue, BoundaryStatusMap)
    //           << " valid nodes in the queue." << std::endl;
    
    // Sometimes pixels that are not in the finishing region that were boundary pixels are no longer
    // boundary pixels after the filling. Check for these.
    // E.g. (H=hole, B=boundary, V=valid, Q=query, F=filled, N=new boundary,
    // R=old boundary pixel that needs to be removed because it is no longer on the boundary)
    // Before filling

    /* V V V B H H H H
     * V V V B H H H H
     * V V V B H H H H
     * V V V B B Q B B
     * V V V V V V V V
     */
    // After filling

    /* V V V B H H H H
     * V V V B H H H H
     * V V V B F F F H
     * V V V B F F F B
     * V V V V F F F V
     */
    // New boundary
    /* V V V B H H H H
     * V V V B H H H H
     * V V V B N N N H
     * V V V R F F N B
     * V V V V F F F V
     */

    // Expand the region
//    std::cout << "InpaintingVisitor::FinishVertex() expand" << std::endl;
    itk::ImageRegion<2> expandedRegion =
        ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, PatchHalfWidth + 1);
    // Make sure the region is entirely inside the image (to allow for target regions near the image boundary)
    expandedRegion.Crop(this->FullRegion);
    std::vector<itk::Index<2> > boundaryPixels =
        ITKHelpers::GetBoundaryPixels(expandedRegion);
//    std::cout << "InpaintingVisitor::FinishVertex() boundary pixels" << std::endl;
    for(unsigned int i = 0; i < boundaryPixels.size(); ++i)
    {
      // the region (the entire image) can be omitted, as this function automatically checks if the pixels are inside the image
      if(!this->MaskImage->HasHoleNeighbor(boundaryPixels[i]))
      {
        VertexDescriptorType v =
            Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(boundaryPixels[i]);

        put(this->BoundaryNodeQueue->BoundaryStatusMap, v, false);
      }
    }

    // std::cout << "FinishVertex after removing stale nodes outside finishing region there are "
    //               << BoostHelpers::CountValidQueueNodes(BoundaryNodeQueue, BoundaryStatusMap)
    //               << " valid nodes in the queue." << std::endl;

    this->NumberOfFinishedPatches++;

//    std::cout << "Leave InpaintingVisitor::FinishVertex()" << std::endl;
  } // end FinishVertex

  void InpaintingComplete() const
  {
    std::cout << "Inpainting complete!" << std::endl;
    // We prefer to write the final image in the calling function, as there we will know what type it is (i.e. do we need to convert back to RGB? etc.)
  }

}; // end InpaintingVisitor

#endif
