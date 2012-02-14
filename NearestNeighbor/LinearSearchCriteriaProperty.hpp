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
  unsigned int DistanceThreshold;
  DistanceFunctionType DistanceFunction;
  CompareFunctionType CompareFunction;

public:
  LinearSearchCriteriaProperty(PropertyMapType propertyMap, const float distanceThreshold, DistanceFunctionType distanceFunction = DistanceFunctionType(), CompareFunctionType compareFunction = CompareFunctionType()) : 
  PropertyMap(propertyMap), DistanceThreshold(distanceThreshold), DistanceFunction(distanceFunction), CompareFunction(compareFunction)
  {
  }

  void SetK(const unsigned int k)
  {
    this->K = k;
  }
  
  unsigned int GetK() const
  {
    return this->K;
  }

  template <typename ForwardIteratorType, typename OutputContainerType>
  void operator()(ForwardIteratorType first, ForwardIteratorType last, typename ForwardIteratorType::value_type queryNode, OutputContainerType& output)
  {
    output.clear();
    if(first == last)
    {
      return;
    }

    std::vector<DistanceValueType> output_dist;
    for(; first != last; ++first)
    {
      DistanceValueType d = DistanceFunction(get(PropertyMap, *first), get(PropertyMap, queryNode));
      
      // If the distance is not less than infinity, it is useless, so do not continue
      if(!CompareFunction(d, std::numeric_limits<DistanceValueType>::infinity()))
      {
        continue;
      }
      if(CompareFunction(d, DistanceThreshold))
      {
        output.push_back(*first);
      }
    }
  }

};

#endif
