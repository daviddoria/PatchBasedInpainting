#ifndef InpaintingVisitor_HPP
#define InpaintingVisitor_HPP

#include "Priority/Priority.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Helpers
#include "Helpers/ITKHelpers.h"
#include "Helpers/HelpersOutput.h"

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It forwards
 * initialize_vertex and discover_vertex, the only two functions that need to know about
 * the descriptor type, to a visitor that models DescriptorVisitorConcept.
 */
template <typename TGraph, typename TImage, typename TBoundaryNodeQueue,
          typename TFillStatusMap, typename TDescriptorVisitor, typename TPriority,
          typename TPriorityMap, typename TBoundaryStatusMap>
struct InpaintingVisitor
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TBoundaryNodeQueue& BoundaryNodeQueue;
  TPriority* PriorityFunction;
  TFillStatusMap& FillStatusMap;
  TDescriptorVisitor& DescriptorVisitor;

  TPriorityMap& PriorityMap;
  TBoundaryStatusMap& BoundaryStatusMap;

  unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  InpaintingVisitor(TImage* const in_image, Mask* const in_mask,
                    TBoundaryNodeQueue& in_boundaryNodeQueue, TFillStatusMap& in_fillStatusMap,
                    TDescriptorVisitor& in_descriptorVisitor, TPriorityMap& in_priorityMap,
                    TPriority* const in_priorityFunction,
                    const unsigned int in_half_width, TBoundaryStatusMap& in_boundaryStatusMap) :
  Image(in_image), MaskImage(in_mask), BoundaryNodeQueue(in_boundaryNodeQueue), PriorityFunction(in_priorityFunction), FillStatusMap(in_fillStatusMap), DescriptorVisitor(in_descriptorVisitor),
  PriorityMap(in_priorityMap), BoundaryStatusMap(in_boundaryStatusMap), HalfWidth(in_half_width), NumberOfFinishedVertices(0)
  {
  }

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  {
    DescriptorVisitor.initialize_vertex(v, g);
  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const
  {
    DescriptorVisitor.discover_vertex(v, g);
  };

  void vertex_match_made(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    std::cout << "Match made: target: " << target[0] << " " << target[1]
              << " with source: " << source[0] << " " << source[1] << std::endl;
    assert(get(fillStatusMap, source));
    assert(get(descriptorMap, source).IsFullyValid());
  };

  void paint_vertex(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    itk::Index<2> target_index;
    target_index[0] = target[0];
    target_index[1] = target[1];

    itk::Index<2> source_index;
    source_index[0] = source[0];
    source_index[1] = source[1];

    assert(image->GetLargestPossibleRegion().IsInside(source_index));
    assert(image->GetLargestPossibleRegion().IsInside(target_index));

    Image->SetPixel(target_index, Image->GetPixel(source_index));
  };

  bool accept_painted_vertex(VertexDescriptorType v, TGraph& g) const
  {
    return true;
  };

  void finish_vertex(VertexDescriptorType v, TGraph& g)
  {
    // Construct the region around the vertex
    itk::Index<2> indexToFinish;
    indexToFinish[0] = v[0];
    indexToFinish[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, HalfWidth);

    region.Crop(Image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Mark all the pixels in this region as filled. This must be done before creating
    // the mask image to use to check for boundary pixels.
    // It does not matter which image we iterate over, we just want the indices.
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(Image, region);

    while(!gridIterator.IsAtEnd())
      {
      VertexDescriptorType v;
      v[0] = gridIterator.GetIndex()[0];
      v[1] = gridIterator.GetIndex()[1];
      put(FillStatusMap, v, true);
      MaskImage->SetPixel(gridIterator.GetIndex(), MaskImage->GetValidValue());
      ++gridIterator;
      }

    // Initialize the newly filled vertices because they may now be valid source nodes.
    gridIterator.GoToBegin();
    while(!gridIterator.IsAtEnd())
      {
      VertexDescriptorType v;
      v[0] = gridIterator.GetIndex()[0];
      v[1] = gridIterator.GetIndex()[1];
      initialize_vertex(v, g);
      ++gridIterator;
      }

    // Update the priority function.
    this->PriorityFunction->Update(indexToFinish);

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(MaskImage, region);

    while(!imageIterator.IsAtEnd())
      {
      VertexDescriptorType v;
      v[0] = imageIterator.GetIndex()[0];
      v[1] = imageIterator.GetIndex()[1];

      // Mark all nodes in the patch around this node as filled (in the FillStatusMap).
      // This makes them ignored if they are still in the boundaryNodeQueue.
      if(ITKHelpers::HasNeighborWithValue(imageIterator.GetIndex(), MaskImage, MaskImage->GetHoleValue()))
        {
        put(BoundaryStatusMap, v, true);
        this->BoundaryNodeQueue.push(v);
        float priority = this->PriorityFunction->ComputePriority(imageIterator.GetIndex());
        //std::cout << "updated priority: " << priority << std::endl;
        put(PriorityMap, v, priority);
        }
      else
        {
        put(BoundaryStatusMap, v, false);
        }

      ++imageIterator;
      }

    {
    // Debug only - write the mask to a file
    HelpersOutput::WriteImage(MaskImage, Helpers::GetSequentialFileName("debugMask", this->NumberOfFinishedVertices, "png"));
    HelpersOutput::WriteVectorImageAsRGB(Image, Helpers::GetSequentialFileName("output", this->NumberOfFinishedVertices, "png"));
    this->NumberOfFinishedVertices++;
    }
  }; // finish_vertex

}; // InpaintingVisitor

#endif
