#ifndef ImagePatchInpaintingVisitor_HPP
#define ImagePatchInpaintingVisitor_HPP

#include "Priority/Priority.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It creates
 * and differences ImagePatch objects at each pixel.
 */
template <typename TImage, typename TBoundaryNodeQueue, typename TFillStatusMap, typename TDescriptorMap, typename TPriorityMap>
struct ImagePatch_inpainting_visitor 
{
  TImage* image;
  TBoundaryNodeQueue* boundaryNodeQueue;
  Priority* priorityFunction;
  TFillStatusMap* fillStatusMap;
  TDescriptorMap* descriptorMap;
  TPriorityMap* priorityMap;

  unsigned int half_width;

  ImagePatch_inpainting_visitor(TImage* const in_image, TBoundaryNodeQueue* const in_boundaryNodeQueue, TFillStatusMap* const in_fillStatusMap, TDescriptorMap* const in_descriptorMap,  
                                TPriorityMap* const in_priorityMap, Priority* const in_priorityFunction, const unsigned int in_half_width) : 
  image(in_image), boundaryNodeQueue(in_boundaryNodeQueue), priorityFunction(in_priorityFunction), fillStatusMap(in_fillStatusMap), descriptorMap(in_descriptorMap), 
  priorityMap(in_priorityMap), half_width(in_half_width)
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
    
    typename boost::property_traits<TDescriptorMap>::value_type descriptor(this->image, region);

    put(*descriptorMap, v, descriptor);

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
  void finish_vertex(VertexType v, Graph& g) const
  {
    // Update the priority function
    // Construct the region around the vertex
    itk::Index<2> index;
    index[0] = v[0];
    index[1] = v[1];

    this->priorityFunction->Update(index);

    // Add all nodes on the boundary of the patch around this node to the boundaryNodeQueue, and update their priority
    for(unsigned int i = v[0] - half_width; i <= v[0] + half_width; ++i)
      {
      VertexType v;
      v[0] = i;
      v[1] = v[1] - half_width;
      this->boundaryNodeQueue->push(v);

      itk::Index<2> index;
      index[0] = v[0];
      index[1] = v[1];
      put(*priorityMap, v, this->priorityFunction->ComputePriority(index));

      v[1] = v[1] + half_width;
      this->boundaryNodeQueue->push(v);

      index[1] = v[1];
      put(*priorityMap, v, this->priorityFunction->ComputePriority(index));

      }

    for(unsigned int j = v[1] - half_width; j <= v[1] + half_width; ++j)
      {
      VertexType v;
      v[0] = v[0] - half_width;;
      v[1] = j;
      this->boundaryNodeQueue->push(v);

      itk::Index<2> index;
      index[0] = v[0];
      index[1] = v[1];
      put(*priorityMap, v, this->priorityFunction->ComputePriority(index));

      v[0] = v[0] + half_width;
      this->boundaryNodeQueue->push(v);

      index[1] = v[1];
      put(*priorityMap, v, this->priorityFunction->ComputePriority(index));
      }

    // TODO: Mark all nodes in the patch around this node as filled (in the FillStatusMap). This makes them ignored if they are still in the boundaryNodeQueue.

  };
};

#endif
