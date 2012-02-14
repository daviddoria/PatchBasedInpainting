#ifndef LinearSearchCriteriaProperty_HPP
#define LinearSearchCriteriaProperty_HPP

// STL
#include <limits> // for infinity()
#include <algorithm> // for lower_bound()

/**
  * This function will search for the compute the difference between a query node's property and
  * all other nodes properties, and return only those which have a difference (float) below a specified
  * threshold.
  * \tparam DistanceValue The value-type for the distance measures.
  * \tparam ForwardIterator The forward-iterator type.
  * \tparam OutputContainer The container type which can contain the list of nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
  * \tparam GetDistanceFunction The functor type to compute the distance measure.
  * \tparam CompareFunction The functor type that can compare two distance measures (strict weak-ordering).
  * \param first Start of the range in which to search.
  * \param last One element past the last element in the range in which to search.
  * \param output The container that will have the sorted list of elements with the smallest distance.
  * \param distance A callable object that returns a DistanceValue for a given element from the ForwardIterator dereferencing.
  * \param compare A callable object that returns true if the first element is the preferred one (less-than) of the two.
  */
template <typename PropertyMapType,
          typename DistanceFunctionType,
          typename DistanceValueType = float,
          typename CompareFunctionType = std::less<DistanceValueType> >
class LinearSearchCriteriaProperty
{
  PropertyMapType PropertyMap;
  float DistanceThreshold;
  DistanceFunctionType DistanceFunction;
  CompareFunctionType CompareFunction;

public:
  LinearSearchCriteriaProperty(PropertyMapType propertyMap, const float distanceThreshold, DistanceFunctionType distanceFunction = DistanceFunctionType(), CompareFunctionType compareFunction = CompareFunctionType()) : 
  PropertyMap(propertyMap), DistanceThreshold(distanceThreshold), DistanceFunction(distanceFunction), CompareFunction(compareFunction)
  {
    std::cout << "DistanceThreshold: " << DistanceThreshold << std::endl;
  }

  void SetDistanceThreshold(const float distanceThreshold)
  {
    this->DistanceThreshold = distanceThreshold;
  }
  
  float GetDistanceThreshold() const
  {
    return this->DistanceThreshold;
  }

  template <typename ForwardIteratorType, typename OutputContainerType>
  void operator()(ForwardIteratorType first, ForwardIteratorType last, typename ForwardIteratorType::value_type queryNode, OutputContainerType& output)
  {
    output.clear();
    if(first == last)
    {
      return;
    }

    for(; first != last; ++first)
    {
      DistanceValueType d = DistanceFunction(get(PropertyMap, *first), get(PropertyMap, queryNode));
      std::cout << "First: " << *first << " : " << get(PropertyMap, *first) << " query: " << queryNode << " : " << get(PropertyMap, queryNode) << std::endl;
      // If the distance is not less than infinity, it is useless, so do not continue
      if(!CompareFunction(d, std::numeric_limits<DistanceValueType>::infinity()))
      {
        continue;
      }

      if(CompareFunction(d, DistanceThreshold))
      {
        std::cout << d << " was less than " << DistanceThreshold << std::endl;
        output.push_back(*first);
      }
      else
      {
        std::cout << d << " was NOT less than " << DistanceThreshold << std::endl;
      }
    }
  }

};

#endif
