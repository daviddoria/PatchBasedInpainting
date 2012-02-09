#ifndef LinearSearchBestProperty_HPP
#define LinearSearchBestProperty_HPP

// STL
#include <limits>

/**
   * This function template is similar to std::min_element but can be used when the comparison
   * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the
   * the element in the range [first,last) which has the "smallest" distance (of course, both the
   * distance metric and comparison can be overriden to perform something other than the canonical
   * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
   * \tparam DistanceValue The value-type for the distance measures.
   * \tparam ForwardIterator The forward-iterator type.
   * \tparam GetDistanceFunction The functor type to compute the distance measure.
   * \tparam CompareFunction The functor type that can compare two distance measures (strict weak-ordering).
   * \param first Start of the range in which to search.
   * \param last One element past the last element in the range in which to search.
   * \param distance A callable object that returns a DistanceValue for a given element from the ForwardIterator dereferencing.
   * \param compare A callable object that returns true if the first element is the preferred one (less-than) of the two.
   * \param inf A DistanceValue which represents infinity (i.e. the very worst value with which to initialize the search).
   * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all the elements in the range with respect to the distance metric).
   */
template <typename DistanceFunctionType,
          typename PropertyMapType,
          typename DistanceValueType = float,
          typename CompareFunctionType = std::less<DistanceValueType> >
struct LinearSearchBestProperty
{
  PropertyMapType PropertyMap;
  DistanceFunctionType DistanceFunction;
  CompareFunctionType CompareFunction;

  LinearSearchBestProperty(PropertyMapType propertyMap, DistanceFunctionType distanceFunction = DistanceFunctionType(), CompareFunctionType compareFunction = CompareFunctionType()) :
  PropertyMap(propertyMap), DistanceFunction(distanceFunction), CompareFunction(compareFunction){}

  template <typename TIterator>
  typename TIterator::value_type operator()(TIterator first, TIterator last, typename TIterator::value_type query)
  {
    if(first == last)
    {
      return *last;
    }

    DistanceValueType d_best = std::numeric_limits<DistanceValueType>::infinity();
    TIterator result = last;
    for(; first != last; ++first)
    {
      //DistanceValueType d = DistanceFunction(*first, query);
      DistanceValueType d = DistanceFunction(get(PropertyMap, *first), get(PropertyMap, query));
      if(CompareFunction(d, d_best))
      {
        d_best = d;
        result = first;
      };
    };
    return *result;
  }
};

#endif
