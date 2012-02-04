#ifndef RandomDescriptorCreatorVisitor_HPP
#define RandomDescriptorCreatorVisitor_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**
 * This is a simplified visitor type that models with the InpaintingVisitorConcept and does
 * nothing but create random descriptors to initialize nodes.
 */
template <typename TDescriptorMap>
struct RandomDescriptorCreatorVisitor
{
  TDescriptorMap& descriptorMap;
  unsigned int dimension;

  RandomDescriptorCreatorVisitor(TDescriptorMap& in_descriptorMap, const unsigned int in_dimension) :
  descriptorMap(in_descriptorMap), dimension(in_dimension)
  {

  }

  template <typename VertexType, typename Graph>
  void initialize_vertex(VertexType v, Graph& g) const
  {
    typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
    DescriptorType descriptor;
    for(unsigned int i = 0; i < dimension; ++i)
      {
      descriptor[i] = drand48();
      }
    put(descriptorMap, v, descriptor);
  };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType, VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType, VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  bool accept_painted_vertex(VertexType, Graph&) const { return true; };

  template <typename VertexType, typename Graph>
  void finish_vertex(VertexType, Graph&) const { };
};

#endif
