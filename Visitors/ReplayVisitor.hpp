#ifndef ReplayVisitor_HPP
#define ReplayVisitor_HPP

#include "Visitors/InpaintingVisitorParent.h"

// Concepts
#include "Concepts/DescriptorVisitorConcept.hpp"

// Priority
#include "Priority/Priority.h"

// Accept criteria
#include "ImageProcessing/BoundaryEnergy.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Helpers
#include "Helpers/ITKHelpers.h"
#include "Helpers/OutputHelpers.h"

/**
 * This is a visitor that replays an inpainting from a log file.
 */
template <typename TGraph, typename TImage, typename TBoundaryNodeQueue,
          typename TFillStatusMap, typename TBoundaryStatusMap>
struct ReplayVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TBoundaryNodeQueue& BoundaryNodeQueue;
  TFillStatusMap& FillStatusMap;
  TBoundaryStatusMap& BoundaryStatusMap;

  const unsigned int HalfWidth;

  ReplayVisitor(TImage* const in_image, Mask* const in_mask,
                TBoundaryNodeQueue& in_boundaryNodeQueue, TFillStatusMap& in_fillStatusMap,
                const unsigned int in_half_width, TBoundaryStatusMap& in_boundaryStatusMap) :
  Image(in_image), MaskImage(in_mask), BoundaryNodeQueue(in_boundaryNodeQueue), 
  FillStatusMap(in_fillStatusMap), BoundaryStatusMap(in_boundaryStatusMap),
  HalfWidth(in_half_width)
  {
  }

  void InitializeVertex(VertexDescriptorType v, TGraph& g) const
  {
    
  };

  void DiscoverVertex(VertexDescriptorType v, TGraph& g) const
  {
    
  };

  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source, TGraph& g)
  {
    
  };

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    itk::Index<2> target_index = ITKHelpers::CreateIndex(target);

    itk::Index<2> source_index = ITKHelpers::CreateIndex(source);

    assert(Image->GetLargestPossibleRegion().IsInside(source_index));
    assert(Image->GetLargestPossibleRegion().IsInside(target_index));

    Image->SetPixel(target_index, Image->GetPixel(source_index));
  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    return true;
  };

  void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode, TGraph& g)
  {
    // Mark this pixel as filled, the area around it as filled, and the mask in this region as filled.
    // Determine the new boundary, and setup the nodes in the boundary queue.

    // Construct the region around the vertex
    itk::Index<2> indexToFinish = ITKHelpers::CreateIndex(v);

    itk::ImageRegion<2> regionToFinish = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, HalfWidth);

    regionToFinish.Crop(Image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Mark all the pixels in this region as filled. This must be done before creating
    // the mask image to use to check for boundary pixels.
    // It does not matter which image we iterate over, we just want the indices.
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(Image, regionToFinish);

    while(!gridIterator.IsAtEnd())
      {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(gridIterator.GetIndex());

      put(FillStatusMap, v, true);
      MaskImage->SetPixel(gridIterator.GetIndex(), MaskImage->GetValidValue());
      ++gridIterator;
      }

    // Initialize the newly filled vertices because they may now be valid source nodes.
    // You may not want to do this in some cases (i.e. if the descriptors needed cannot be
    // computed on newly filled regions)
    gridIterator.GoToBegin();
    while(!gridIterator.IsAtEnd())
      {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(gridIterator.GetIndex());

      InitializeVertex(v, g);
      ++gridIterator;
      }

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(MaskImage, regionToFinish);

    while(!imageIterator.IsAtEnd())
      {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(imageIterator.GetIndex());

      // Mark all nodes in the patch around this node as filled (in the FillStatusMap).
      // This makes them ignored if they are still in the boundaryNodeQueue.
      if(ITKHelpers::HasNeighborWithValue(imageIterator.GetIndex(), MaskImage, MaskImage->GetHoleValue()))
        {
        put(BoundaryStatusMap, v, true);
        this->BoundaryNodeQueue.push(v);
        }
      else
        {
        put(BoundaryStatusMap, v, false);
        }

      ++imageIterator;
      }

  }; // finish_vertex

  void InpaintingComplete() const
  {
    HelpersOutput::WriteImage(Image, "output.mha");
  }

}; // Replay

#endif
