#ifndef SearchFunctor_HPP
#define SearchFunctor_HPP

#include "LinearSearchAlgorithm.hpp"

template <typename TGraph, typename TDistanceFunction, typename TDescriptorMap>
struct SearchFunctor
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;
  TDistanceFunction DistanceFunction;
  TGraph Graph;
  TDescriptorMap DescriptorMap;

  SearchFunctor(TGraph& g, TDescriptorMap& descriptorMap) : Graph(g), DescriptorMap(descriptorMap){}

  VertexDescriptorType operator()(const VertexDescriptorType& queryVertexDescriptor)
  {
    typedef typename boost::graph_traits<TGraph>::vertex_iterator VertexIteratorType;
    VertexIteratorType ui,ui_end; tie(ui,ui_end) = vertices(Graph);

    VertexDescriptorType result = LinearSearchPropertyBest(ui, ui_end,
                                                         DistanceFunction, DescriptorMap, queryVertexDescriptor);
    return result;
  }
};

#endif
