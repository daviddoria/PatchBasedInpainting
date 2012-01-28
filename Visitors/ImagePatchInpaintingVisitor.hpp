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
  TBoundaryNodeQueue* boundaryNodeQueue;
  Priority* priorityFunction;
  TFillStatusMap* fillStatusMap;
  TDescriptorMap* descriptorMap;
  TPriorityMap* priorityMap;
  TBoundaryStatusMap* boundaryStatusMap;

  unsigned int half_width;
  unsigned int NumberOfFinishedVertices;

  ImagePatch_inpainting_visitor(TImage* const in_image, TBoundaryNodeQueue* const in_boundaryNodeQueue, TFillStatusMap* const in_fillStatusMap,
                                TDescriptorMap* const in_descriptorMap, TPriorityMap* const in_priorityMap, Priority* const in_priorityFunction,
                                const unsigned int in_half_width, TBoundaryStatusMap* in_boundaryStatusMap) :
  image(in_image), boundaryNodeQueue(in_boundaryNodeQueue), priorityFunction(in_priorityFunction), fillStatusMap(in_fillStatusMap), descriptorMap(in_descriptorMap),
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

    if(image->GetLargestPossibleRegion().IsInside(region))
    {
      DescriptorType descriptor(this->image, region);
      put(*descriptorMap, v, descriptor);
    }
    else
    {
      // The region is not entirely inside the image so it cannot be used as a source patch
      DescriptorType descriptor; // This descriptor is invalid, so any comparison to it will return infinity.
      put(*descriptorMap, v, descriptor);
    }

  };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType v, Graph& g) const
  {
    // QUESTION: what would be done in this function?
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
    // Update the priority function
    // Construct the region around the vertex
    itk::Index<2> indexToFinish;
    indexToFinish[0] = v[0];
    indexToFinish[1] = v[1];

    // Create a mask image from the fillStatusMap. We need to to determine if pixels have hole neighbors.
    Mask::Pointer mask = Mask::New();
    mask->SetRegions(image->GetLargestPossibleRegion());
    mask->Allocate();

    itk::ImageRegionIterator<Mask> maskIterator(mask, mask->GetLargestPossibleRegion());

    while(!maskIterator.IsAtEnd())
      {
      VertexType v;
      v[0] = maskIterator.GetIndex()[0];
      v[1] = maskIterator.GetIndex()[1];
      bool fillStatus = get(*fillStatusMap, v);
      if(fillStatus)
        {
        maskIterator.Set(mask->GetValidValue());
        }
      else
        {
        maskIterator.Set(mask->GetHoleValue());
        }

      ++maskIterator;
      }

    this->priorityFunction->Update(indexToFinish);

    // Mark all nodes in the patch around this node as filled (in the FillStatusMap). This makes them ignored if they are still in the boundaryNodeQueue.
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, half_width);

    region.Crop(image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(mask, region);

    while(!imageIterator.IsAtEnd())
      {
      VertexType v;
      v[0] = imageIterator.GetIndex()[0];
      v[1] = imageIterator.GetIndex()[1];
      put(*fillStatusMap, v, true);

      if(ITKHelpers::HasNeighborWithValue(imageIterator.GetIndex(), mask.GetPointer(), mask->GetHoleValue()))
        {
        put(*boundaryStatusMap, v, true);
        this->boundaryNodeQueue->push(v);
        put(*priorityMap, v, this->priorityFunction->ComputePriority(imageIterator.GetIndex()));
        }
      else
        {
        put(*boundaryStatusMap, v, false);
        }

      ++imageIterator;
      }

    {
    // Debug only - write the mask to a file
    HelpersOutput::WriteImage(mask.GetPointer(), Helpers::GetSequentialFileName("debugMask", this->NumberOfFinishedVertices, "png"));
    this->NumberOfFinishedVertices++;
    }
  }; // finish_vertex

}; // ImagePatch_inpainting_visitor

#endif
