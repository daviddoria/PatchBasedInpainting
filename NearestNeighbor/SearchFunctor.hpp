#ifndef SearchFunctor_HPP
#define SearchFunctor_HPP

#include "LinearSearchAlgorithm.hpp"

template <typename TVertexDescriptor, typename TObject, typename TDistanceFunction, typename TDescriptorMap>
struct SearchFunctor
{
  typedef std::vector<TObject> ObjectContainer;
  ObjectContainer Objects;
  TDistanceFunction DistanceFunction;

  TDescriptorMap DescriptorMap;

  SearchFunctor(TDescriptorMap& descriptorMap) : DescriptorMap(descriptorMap){}

  typename ObjectContainer::iterator operator()(const TObject& query)
  {
    return LinearSearchBest(Objects.begin(), Objects.end(), DistanceFunction);
  }

  typename ObjectContainer::iterator operator()(const TVertexDescriptor& query)
  {
    TObject object = get(DescriptorMap, query);
    DistanceFunction.
    return LinearSearchBest(Objects.begin(), Objects.end(), DistanceFunction);
  }
};

#endif
