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

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It forwards
 * initialize_vertex and discover_vertex, the only two functions that need to know about
 * the descriptor type, to a visitor that models DescriptorVisitorConcept.
 * The visitor needs to know the patch size of the patch to be inpainted because it uses
 * this size to traverse the inpainted region to update the boundary.
 */
template <typename TGraph, typename TImage, typename TBoundaryNodeQueue,
          typename TDescriptorVisitor, typename TAcceptanceVisitor, typename TPriority,
          typename TPriorityMap, typename TBoundaryStatusMap>
struct InpaintingVisitor : public InpaintingVisitorParent<TGraph>
{
  BOOST_CONCEPT_ASSERT((DescriptorVisitorConcept<TDescriptorVisitor, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TBoundaryNodeQueue& BoundaryNodeQueue;
  TPriority* PriorityFunction;
  TDescriptorVisitor& DescriptorVisitor;
  TAcceptanceVisitor& AcceptanceVisitor;

  TPriorityMap& PriorityMap;
  TBoundaryStatusMap& BoundaryStatusMap;

  const unsigned int PatchHalfWidth;

  std::string ResultFileName;

  unsigned int NumberOfFinishedPatches;

  InpaintingVisitor(TImage* const image, Mask* const mask,
                    TBoundaryNodeQueue& boundaryNodeQueue,
                    TDescriptorVisitor& descriptorVisitor, TAcceptanceVisitor& acceptanceVisitor,
                    TPriorityMap& priorityMap, TPriority* const priorityFunction,
                    const unsigned int patchHalfWidth, TBoundaryStatusMap& boundaryStatusMap,
                    const std::string& resultFileName,
                    const std::string& visitorName = "InpaintingVisitor") :
  InpaintingVisitorParent<TGraph>(visitorName),
  Image(image), MaskImage(mask), BoundaryNodeQueue(boundaryNodeQueue), PriorityFunction(priorityFunction),
  DescriptorVisitor(descriptorVisitor), AcceptanceVisitor(acceptanceVisitor),
  PriorityMap(priorityMap), BoundaryStatusMap(boundaryStatusMap),
  PatchHalfWidth(patchHalfWidth), ResultFileName(resultFileName), NumberOfFinishedPatches(0)
  {
  }

  void InitializeVertex(VertexDescriptorType v) const
  {
    DescriptorVisitor.InitializeVertex(v);
  }

  void DiscoverVertex(VertexDescriptorType v) const
  {
    DescriptorVisitor.DiscoverVertex(v);
  }

  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source)
  {
    // Debug only checks
    //assert(get(FillStatusMap, source));
    assert(ITKHelpers::HasNeighborWithValue(ITKHelpers::CreateIndex(target), MaskImage, MaskImage->GetHoleValue()));

    // For faster debugging (so we can "printf debug" in release mode), we make these cheks at runtime as well
//     if(!get(FillStatusMap, source))
//     {
//       std::stringstream ss;
//       ss << "Potential source pixel " << source[0] << " " << source[1]
//          << " does not have 'true' value in FillStatusMap!";
//       throw std::runtime_error(ss.str());
//     }
    if(!MaskImage->IsValid(ITKHelpers::CreateIndex(source)))
    {
      std::stringstream ss;
      ss << "Potential source pixel " << source[0] << " " << source[1] << " is not valid in the mask!";
      throw std::runtime_error(ss.str());
    }

    if(!ITKHelpers::HasNeighborWithValue(ITKHelpers::CreateIndex(target), MaskImage, MaskImage->GetHoleValue()))
    {
      std::stringstream ss;
      ss << "Potential target pixel " << target[0] << " " << target[1] << " does not have a hole neighbor!";
      throw std::runtime_error(ss.str());
    }
  }

  void PaintPatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    VertexDescriptorType target_patch_corner;
    target_patch_corner[0] = target[0] - PatchHalfWidth;
    target_patch_corner[1] = target[1] - PatchHalfWidth;

    VertexDescriptorType source_patch_corner;
    source_patch_corner[0] = source[0] - PatchHalfWidth;
    source_patch_corner[1] = source[1] - PatchHalfWidth;

    VertexDescriptorType target_node;
    VertexDescriptorType source_node;
    for(std::size_t i = 0; i < PatchHalfWidth * 2 + 1; ++i)
    {
      for(std::size_t j = 0; j < PatchHalfWidth * 2 + 1; ++j)
      {
        target_node[0] = target_patch_corner[0] + i;
        target_node[1] = target_patch_corner[1] + j;

        source_node[0] = source_patch_corner[0] + i;
        source_node[1] = source_patch_corner[1] + j;

        // Only paint the pixel if it is currently a hole
        if( MaskImage->IsHole(ITKHelpers::CreateIndex(target_node)) )
        {
          //std::cout << "Copying pixel " << source_node << " to pixel " << target_node << std::endl;
          PaintVertex(target_node, source_node); //paint the vertex.
        }

      }
    }
  };
  
  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const
  {
    itk::Index<2> target_index = ITKHelpers::CreateIndex(target);

    itk::Index<2> source_index = ITKHelpers::CreateIndex(source);

    assert(Image->GetLargestPossibleRegion().IsInside(source_index));
    assert(Image->GetLargestPossibleRegion().IsInside(target_index));

    Image->SetPixel(target_index, Image->GetPixel(source_index));
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    float energy = 0.0f;
    return AcceptanceVisitor.AcceptMatch(target, source, energy);
  }

  void FinishVertex(VertexDescriptorType targetNode, VertexDescriptorType sourceNode)
  {
    // Mark this pixel and the area around it as filled, and mark the mask in this region as filled.
    // Determine the new boundary, and setup the nodes in the boundary queue.

    // Construct the region around the vertex
    itk::Index<2> indexToFinish = ITKHelpers::CreateIndex(targetNode);

    itk::ImageRegion<2> regionToFinish = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, PatchHalfWidth);

    regionToFinish.Crop(Image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Update the priority function. This must be done BEFORE the mask is filled,
    // as the old mask is required in some of the Priority functors to determine where to update some things.
    this->PriorityFunction->Update(sourceNode, targetNode, this->NumberOfFinishedPatches);

    // Mark all the pixels in this region as filled (in the mask and in the FillStatusMap).
    {
    itk::ImageRegionIteratorWithIndex<Mask> maskIterator(MaskImage, regionToFinish);

    while(!maskIterator.IsAtEnd())
      {
      MaskImage->MarkAsValid(maskIterator.GetIndex());
      ++maskIterator;
      }
    }

    // Initialize all vertices in the newly filled region because they may now be valid source nodes.
    // (You may not want to do this in some cases (i.e. if the descriptors needed cannot be 
    // computed on newly filled regions))
    {
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(Image, regionToFinish);
    while(!gridIterator.IsAtEnd())
      {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(gridIterator.GetIndex());

      InitializeVertex(v);
      ++gridIterator;
      }
    }

    {
    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(MaskImage, regionToFinish);

    while(!imageIterator.IsAtEnd())
      {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(imageIterator.GetIndex());

      // This makes them ignored if they are still in the boundaryNodeQueue.
      if(MaskImage->HasHoleNeighbor(imageIterator.GetIndex()))
        {
        // Note: must set the value in the priority map before pushing the node
        // into the queue (as the priority is what
        // determines the node's position in the queue).
        float priority = this->PriorityFunction->ComputePriority(imageIterator.GetIndex());
        //std::cout << "updated priority: " << priority << std::endl;
        put(PriorityMap, v, priority);

        put(BoundaryStatusMap, v, true);
        this->BoundaryNodeQueue.push(v);
        }
      else
        {
        put(BoundaryStatusMap, v, false);
        }

      ++imageIterator;
      }
    }

    // std::cout << "FinishVertex after traversing finishing region there are "
    //           << BoostHelpers::CountValidQueueNodes(BoundaryNodeQueue, BoundaryStatusMap)
    //           << " valid nodes in the queue." << std::endl;
    
    // Sometimes pixels that are not in the finishing region that were boundary pixels are no longer
    // boundary pixels after the filling. Check for these.
    {
    // Expand the region
    itk::ImageRegion<2> expandedRegion = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, PatchHalfWidth + 1);
    std::vector<itk::Index<2> > boundaryPixels = ITKHelpers::GetBoundaryPixels(expandedRegion);
    for(unsigned int i = 0; i < boundaryPixels.size(); ++i)
      {
      if(!ITKHelpers::HasNeighborWithValue(boundaryPixels[i], MaskImage, MaskImage->GetHoleValue()))
        {
        VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(boundaryPixels[i]);
        put(BoundaryStatusMap, v, false);
        }
      }
    }
// std::cout << "FinishVertex after removing stale nodes outside finishing region there are "
//               << BoostHelpers::CountValidQueueNodes(BoundaryNodeQueue, BoundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;

  NumberOfFinishedPatches++;
  } // finish_vertex

  void InpaintingComplete() const
  {
    // If the output filename is a png file, then use the RGBImage writer so that it is first
    // casted to unsigned char. Otherwise, write the file directly.
    if(Helpers::GetFileExtension(this->ResultFileName) == "png")
    {
      ITKHelpers::WriteRGBImage(this->Image, this->ResultFileName);
    }
    else
    {
      ITKHelpers::WriteImage(this->Image, this->ResultFileName);
    }
  }

}; // InpaintingVisitor

#endif
