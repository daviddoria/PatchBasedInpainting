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

// Concepts
#include "Concepts/DescriptorVisitorConcept.hpp"

// Accept criteria
#include "ImageProcessing/BoundaryEnergy.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Helpers
#include "ITKHelpers/ITKHelpers.h"
#include "BoostHelpers/BoostHelpers.h"

// Submodules
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
          typename TDescriptorVisitor, typename TAcceptanceVisitor, typename TPriority,
          typename TImage = itk::Image<unsigned char, 2> >
class InpaintingVisitor : public InpaintingVisitorParent<TGraph>, public Debug
{
  BOOST_CONCEPT_ASSERT((DescriptorVisitorConcept<TDescriptorVisitor, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  /** The image to inpaint. */
  TImage* Image;

  /** The mask indicating the region to inpaint. */
  Mask* MaskImage;

  /** A queue to use to determine which patch to inpaint next. */
  TBoundaryNodeQueue& BoundaryNodeQueue;

  /** A function to determine the priority of each target patch. */
  TPriority* PriorityFunction;

  /** A visitor to do patch descriptor specific operations. */
  TDescriptorVisitor& DescriptorVisitor;

  /** A visitor to perform specified actions when a match is accepted. */
  TAcceptanceVisitor& AcceptanceVisitor;

  /** The radius of the patches to use to inpaint the image. */
  const unsigned int PatchHalfWidth;

  /** How many patches have been finished so far. */
  unsigned int NumberOfFinishedPatches;

  /** As the image is inpainted, this flag determines if new source patches can be created from patches which are now valid. */
  bool AllowNewPatches;

  /** The full region (the mask and image should both be this size). */
  itk::ImageRegion<2> FullRegion;

  /** Track which nodes have been used. */
  typedef std::set<itk::Index<2>, itk::Index<2>::LexicographicCompare> UsedNodesSetType;
  UsedNodesSetType UsedNodesSet;

public:

  UsedNodesSetType* GetUsedNodesSetPointer()
  {
    return &UsedNodesSet;
  }

  void SetAllowNewPatches(const bool allowNewPatches)
  {
    this->AllowNewPatches = allowNewPatches;
  }

  /** Constructor. Everything must be specified in this constructor. (There is no default constructor). */
  InpaintingVisitor(Mask* const mask,
                    TBoundaryNodeQueue& boundaryNodeQueue,
                    TDescriptorVisitor& descriptorVisitor, TAcceptanceVisitor& acceptanceVisitor,
                    TPriority* const priorityFunction,
                    const unsigned int patchHalfWidth,
                    const std::string& visitorName = "InpaintingVisitor", TImage* const image = nullptr) :
    InpaintingVisitorParent<TGraph>(visitorName),
    Image(image), MaskImage(mask), BoundaryNodeQueue(boundaryNodeQueue), PriorityFunction(priorityFunction),
    DescriptorVisitor(descriptorVisitor), AcceptanceVisitor(acceptanceVisitor),
    PatchHalfWidth(patchHalfWidth), NumberOfFinishedPatches(0), AllowNewPatches(false)
  {
    this->FullRegion = this->MaskImage->GetLargestPossibleRegion();
  }

  void InitializeVertex(VertexDescriptorType v) const
  {
    this->DescriptorVisitor.InitializeVertex(v);
  }

  void DiscoverVertex(VertexDescriptorType v) const
  {
    this->DescriptorVisitor.DiscoverVertex(v);
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
    return AcceptanceVisitor.AcceptMatch(target, source, energy);
  }

  /** The mask is inpainted with ValidValue in this function. */
  void FinishVertex(VertexDescriptorType targetNode, VertexDescriptorType sourceNode)// __attribute__((optimize(0))) // This supposedly makes this function build in debug mode (-g -O0) when the rest of the program is built in -O3 or similar)
  {
    // Mark this pixel and the area around it as filled, and mark the mask in this region as filled.
    // Determine the new boundary, and setup the nodes in the boundary queue.

//    std::cout << "InpaintingVisitor::FinishVertex()" << std::endl;

    // Construct the region around the vertex
    itk::Index<2> indexToFinish = ITKHelpers::CreateIndex(targetNode);

    this->UsedNodesSet.insert(indexToFinish);

    itk::ImageRegion<2> regionToFinishFull = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, this->PatchHalfWidth);

    // Copy this region so that we can change (crop) the regionToFinish and still have a copy of the original region
    itk::ImageRegion<2> regionToFinish = regionToFinishFull;

    // Make sure the region is entirely inside the image
    // (because we allow target patches to not be entirely inside the image to handle the case where
    // the hole boundary is near the image boundary)
    regionToFinish.Crop(this->FullRegion);

    if(this->DebugImages)
    {
      ITKHelpers::WriteImage(this->MaskImage, Helpers::GetSequentialFileName("Mask_Before", this->NumberOfFinishedPatches, "png", 3));
    }

    // Mark all the pixels in this region as filled in the mask.
    ITKHelpers::SetRegionToConstant(this->MaskImage, regionToFinish, this->MaskImage->GetValidValue());

    if(this->DebugImages && this->Image)
    {
      ITKHelpers::WriteImage(this->MaskImage, Helpers::GetSequentialFileName("Mask_After", this->NumberOfFinishedPatches, "png", 3));

      typename TImage::PixelType red;
      red.Fill(0);
      red[0] = 255;

      typename TImage::PixelType green;
      green.Fill(0);
      green[1] = 255;

      typename TImage::Pointer patchesCopiedImage = TImage::New();
      ITKHelpers::DeepCopy(this->Image, patchesCopiedImage.GetPointer());
      ITKHelpers::OutlineRegion(patchesCopiedImage.GetPointer(), regionToFinish, red);

      itk::Index<2> sourceRegionCenter = ITKHelpers::CreateIndex(sourceNode);
      itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceRegionCenter, this->PatchHalfWidth);

      sourceRegion = ITKHelpers::CropRegionAtPosition(sourceRegion, this->Image->GetLargestPossibleRegion(), regionToFinishFull);

      ITKHelpers::OutlineRegion(patchesCopiedImage.GetPointer(), sourceRegion, green);

      ITKHelpers::WriteRGBImage(patchesCopiedImage.GetPointer(), Helpers::GetSequentialFileName("PatchesCopied", this->NumberOfFinishedPatches, "png", 3));
    }

    // Update the priority function. This must be done AFTER the mask is filled,
    // as some of the Priority functors only compute things on the hole boundary, or only
    // use data from the valid region of the image (indicated by valid pixels in the mask).
    this->PriorityFunction->Update(sourceNode, targetNode, this->NumberOfFinishedPatches);

    // Initialize (if requested) all vertices in the newly filled region because they may now be valid source nodes.
    // (You may not want to do this in some cases (i.e. if the descriptors needed cannot be
    // computed on newly filled regions))

    if(this->AllowNewPatches)
    {
      std::cout << "Initializing new vertices..." << std::endl;
      itk::ImageRegionConstIteratorWithIndex<Mask> gridIterator(this->MaskImage, regionToFinish);
      while(!gridIterator.IsAtEnd())
      {
        VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(gridIterator.GetIndex());

        InitializeVertex(v);
        ++gridIterator;
      }
    }

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(this->MaskImage, regionToFinish);

    typedef typename TBoundaryNodeQueue::HandleType HandleType;

    while(!imageIterator.IsAtEnd())
    {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(imageIterator.GetIndex());

      if(this->MaskImage->HasHoleNeighbor(imageIterator.GetIndex()))
      {
        float priority = this->PriorityFunction->ComputePriority(imageIterator.GetIndex());
        this->BoundaryNodeQueue.push_or_update(v, priority);
      }
      else
      {
        this->BoundaryNodeQueue.mark_as_invalid(v);
      }

      ++imageIterator;
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
    itk::ImageRegion<2> expandedRegion = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, PatchHalfWidth + 1);
    // Make sure the region is entirely inside the image (to allow for target regions near the image boundary)
    expandedRegion.Crop(this->FullRegion);
    std::vector<itk::Index<2> > boundaryPixels = ITKHelpers::GetBoundaryPixels(expandedRegion);
    for(unsigned int i = 0; i < boundaryPixels.size(); ++i)
    {
      // the region (the entire image) can be omitted, as this function automatically checks if the pixels are inside the image
      if(!this->MaskImage->HasHoleNeighbor(boundaryPixels[i]))
      {
        VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(boundaryPixels[i]);

        put(this->BoundaryNodeQueue.BoundaryStatusMap, v, false);
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
