#ifndef DescriptorConcept_HPP
#define DescriptorConcept_HPP

#include <boost/graph/graph_concepts.hpp>
#include <boost/concept_check.hpp>

#include "PixelDescriptors/PixelDescriptor.h"

/**
 * This concept-check class defines the functions that a descriptor must have in order to be 
 * used by the inpainting algorithms.
 * 
 * Valid Expressions:
 * 
 *  descriptor.SetVertex(v);  Store the vertex that it is describing
 * 
 *  vis.SetStatus(StatusEnum);  Store if it is a source, target, or invalid descriptor.
 *
 * \tparam TDescriptor The descriptor type.
 * \tparam VertexListGraphType The type of the graph that the descriptor describes a vertex of.
 */
template <typename TDescriptor, typename VertexListGraphType>
struct DescriptorConcept 
{
  typedef typename boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptor;
  VertexDescriptor v;
  TDescriptor descriptor;
  BOOST_CONCEPT_USAGE(DescriptorConcept ) 
  {
    descriptor.SetVertex(v);
    descriptor.SetStatus(PixelDescriptor::INVALID);
  };

};

#endif
