#ifndef SearchFunctor_HPP
#define SearchFunctor_HPP

#include "LinearSearchAlgorithm.hpp"

template <typename TVertexDescriptor, typename TObject, typename TDistanceFunction, typename TDescriptorMap>
struct SearchFunctor
{
  typedef std::vector<TObject> ContainerType;
  ContainerType Objects;
  TDistanceFunction DistanceFunction;

  TDescriptorMap DescriptorMap;

  SearchFunctor(TDescriptorMap& descriptorMap) : DescriptorMap(descriptorMap){}

//   TVertexDescriptor operator()(const TObject& query)
//   {
//     typename ObjectContainer::iterator result = LinearSearchBest(Objects.begin(), Objects.end(), DistanceFunction, query);
//     return *result;
//   }

  TVertexDescriptor operator()(const TVertexDescriptor& query)
  {
    TObject object = get(DescriptorMap, query);
    typename ContainerType::iterator result = LinearSearchBest<ContainerType>(Objects.begin(),
                                                                              Objects.end(), DistanceFunction, object);
    ContainerType::iterator it = find(Objects.begin(), Objects.end(), *result);
    // linearId = it - Objects.begin()
    return *result;
  }
};

#endif
