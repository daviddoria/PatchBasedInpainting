#ifndef ImagePatchInpaintingVisitor_HPP
#define ImagePatchInpaintingVisitor_HPP

#include "Priority/Priority.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Helpers
#include "Helpers/ITKHelpers.h"

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It creates
 * and differences ImagePatch objects at each pixel.
 */
template <typename TImage, typename TBoundaryNodeQueue, typename TFillStatusMap, typename TDescriptorMap, typename TPriorityMap, typename TBoundaryStatusMap>
struct ImagePatch_inpainting_visitor 
{
  TImage* image;
  Mask* mask;
  TBoundaryNodeQueue& boundaryNodeQueue;
  Priority* priorityFunction;
  TFillStatusMap& fillStatusMap;
  TDescriptorMap& descriptorMap;
  TPriorityMap& priorityMap;
  TBoundaryStatusMap& boundaryStatusMap;

  unsigned int half_width;
  unsigned int NumberOfFinishedVertices;

  ImagePatch_inpainting_visitor(TImage* const in_image, Mask* const in_mask, TBoundaryNodeQueue& in_boundaryNodeQueue, TFillStatusMap& in_fillStatusMap,
                                TDescriptorMap& in_descriptorMap, TPriorityMap& in_priorityMap, Priority* const in_priorityFunction,
                                const unsigned int in_half_width, TBoundaryStatusMap& in_boundaryStatusMap) :
  image(in_image), mask(in_mask), boundaryNodeQueue(in_boundaryNodeQueue), priorityFunction(in_priorityFunction), fillStatusMap(in_fillStatusMap), descriptorMap(in_descriptorMap),
  priorityMap(in_priorityMap), boundaryStatusMap(in_boundaryStatusMap), half_width(in_half_width), NumberOfFinishedVertices(0)
  {
  }

  template <typename VertexType, typename Graph>
  void initialize_vertex(VertexType v, Graph& g) const
  {
    // Create the patch object and associate with the node
    itk::Index<2> index;
    index[0] = v[0];
    index[1] = v[1];
    itk::Size<2> regionSize;
    regionSize.Fill(half_width);
    itk::ImageRegion<2> region(index, regionSize);

    typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;

    bool valid = image->GetLargestPossibleRegion().IsInside(region) && mask->IsValid(region) && mask->IsValid(index);
    DescriptorType descriptor(this->image, region, valid);
    put(descriptorMap, v, descriptor);

  };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType v, Graph& g) const
  {
    // QUESTION: what would be done in this function?
    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
  };

  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType target, VertexType source, Graph& g) const
  {
    std::cout << "Filled " << target[0] << " " << target[1] << " with " << source[0] << " " << source[1] << std::endl;
  };

  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType target, VertexType source, Graph& g) const
  {
    itk::Index<2> target_index;
    target_index[0] = target[0];
    target_index[1] = target[1];

    itk::Index<2> source_index;
    source_index[0] = source[0];
    source_index[1] = source[1];

    image->SetPixel(target_index, image->GetPixel(source_index));
  };

  template <typename VertexType, typename Graph>
  bool accept_painted_vertex(VertexType v, Graph& g) const
  {
    // QUESTION: What would be done in this function?
    return true;
  };

  template <typename VertexType, typename Graph>
  void finish_vertex(VertexType v, Graph& g)
  {
    // Construct the region around the vertex
    itk::Index<2> indexToFinish;
    indexToFinish[0] = v[0];
    indexToFinish[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, half_width);

    region.Crop(image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Mark all the pixels in this region as filled. This must be done before creating the mask image to use to check for boundary pixels.
    // It does not matter which image we iterate over, we just want the indices. Additionally, initialize these vertices because they may now be valid.
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(image, region);

    while(!gridIterator.IsAtEnd())
      {
      VertexType v;
      v[0] = gridIterator.GetIndex()[0];
      v[1] = gridIterator.GetIndex()[1];
      put(fillStatusMap, v, true);
      mask->SetPixel(gridIterator.GetIndex(), mask->GetValidValue());
      initialize_vertex(v, g);
      ++gridIterator;
      }

    // Update the priority function.
    this->priorityFunction->Update(indexToFinish);

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(mask, region);

    while(!imageIterator.IsAtEnd())
      {
      VertexType v;
      v[0] = imageIterator.GetIndex()[0];
      v[1] = imageIterator.GetIndex()[1];

      // Mark all nodes in the patch around this node as filled (in the FillStatusMap).
      // This makes them ignored if they are still in the boundaryNodeQueue.
      if(ITKHelpers::HasNeighborWithValue(imageIterator.GetIndex(), mask, mask->GetHoleValue()))
        {
        put(boundaryStatusMap, v, true);
        this->boundaryNodeQueue.push(v);
        put(priorityMap, v, this->priorityFunction->ComputePriority(imageIterator.GetIndex()));
        }
      else
        {
        put(boundaryStatusMap, v, false);
        }

      ++imageIterator;
      }

    {
    // Debug only - write the mask to a file
    HelpersOutput::WriteImage(mask, Helpers::GetSequentialFileName("debugMask", this->NumberOfFinishedVertices, "png"));
    this->NumberOfFinishedVertices++;
    }
  }; // finish_vertex

}; // ImagePatch_inpainting_visitor

#endif
