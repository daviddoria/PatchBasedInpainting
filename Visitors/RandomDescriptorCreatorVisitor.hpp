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
  TDescriptorMap& DescriptorMap;
  unsigned int Dimension;

  RandomDescriptorCreatorVisitor(TDescriptorMap& descriptorMap, const unsigned int dimension) :
  DescriptorMap(descriptorMap), Dimension(dimension)
  {

  }

  template <typename VertexType, typename Graph>
  void InitializeVertex(VertexType v, Graph& g) const
  {
    typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
    DescriptorType descriptor;
    for(unsigned int i = 0; i < dimension; ++i)
    {
      descriptor[i] = drand48();
    }
    put(this->DescriptorMap, v, descriptor);
  }

};

#endif
