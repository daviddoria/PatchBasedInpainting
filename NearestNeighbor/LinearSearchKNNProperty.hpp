#ifndef LinearSearchKNNProperty_HPP
#define LinearSearchKNNProperty_HPP

// STL
#include <limits> // for infinity()
#include <algorithm> // for lower_bound()

// Boost
#include <boost/utility.hpp> // for enable_if()
#include <boost/type_traits.hpp> // for is_same()

// Custom
#include "Utilities/Utilities.hpp"

/**
  * This function template is similar to std::min_element but can be used when the comparison
  * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the
  * the elements in the range [first,last) with the "smallest" distances (of course, both the
  * distance metric and comparison can be overriden to perform something other than the canonical
  * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
  * This function will fill the output container with a number of nearest-neighbors.
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
  * \param max_neighbors The maximum number of elements of smallest distance to output in the sorted list.
  */
template <typename PropertyMapType,
          typename DistanceFunctionType,
          typename DistanceValueType = float,
          typename CompareFunctionType = std::less<DistanceValueType> >
class LinearSearchKNNProperty
{
  PropertyMapType PropertyMap;
  unsigned int K;
  DistanceFunctionType DistanceFunction;
  CompareFunctionType CompareFunction;

  unsigned int StrideLength;
public:
  LinearSearchKNNProperty(PropertyMapType propertyMap, const unsigned int k = 1000,
                          const unsigned int strideLength = 1,
                          DistanceFunctionType distanceFunction = DistanceFunctionType(),
                          CompareFunctionType compareFunction = CompareFunctionType()) :
  PropertyMap(propertyMap), K(k), DistanceFunction(distanceFunction),
  CompareFunction(compareFunction), StrideLength(strideLength)
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

  //   template <typename ForwardIteratorType, typename OutputContainerType>
  //   void operator()(ForwardIteratorType first, ForwardIteratorType last,
  //                   typename ForwardIteratorType::value_type queryNode, OutputContainerType& output)

  template <typename ForwardIterator, typename OutputIterator, typename CompareFunction = std::less<float>, typename DistanceValue = float>
  inline
  OutputIterator operator()(ForwardIterator first,
                            ForwardIterator last,
                            typename ForwardIterator::value_type queryNode,
                            OutputIterator output_first,
                            CompareFunction compare = CompareFunction(),
                            DistanceValue radius = std::numeric_limits<DistanceValue>::infinity())
  {

    if(first == last) return output_first;
    std::priority_queue< std::pair<DistanceValue, ForwardIterator>,
                          std::vector< std::pair<DistanceValue, ForwardIterator> >,
                          compare_pair_first<DistanceValue, ForwardIterator, CompareFunction> >
      output_queue = std::priority_queue< std::pair<DistanceValue, ForwardIterator>,
                          std::vector< std::pair<DistanceValue, ForwardIterator> >,
                          compare_pair_first<DistanceValue, ForwardIterator, CompareFunction> >(compare_pair_first<DistanceValue, ForwardIterator, CompareFunction>(compare));
    for(; first != last; ++first) {
      DistanceValue d = DistanceFunction(get(PropertyMap, *first), get(PropertyMap, queryNode));
      if(!compare(d, radius))
        continue;
      output_queue.push(std::pair<DistanceValue, ForwardIterator>(d, first));
      while(output_queue.size() > this->K)
        output_queue.pop();
      radius = output_queue.top().first;
    };

    return copy_neighbors_from_queue<ForwardIterator, DistanceValue>(output_queue, output_first);
  };

  template <typename T1, typename T2, typename Compare>
  struct compare_pair_first : std::binary_function< std::pair<T1, T2>, std::pair<T1, T2>, bool> {
    Compare comp;
    compare_pair_first(const Compare& aComp = Compare()) : comp(aComp) { };
    bool operator()(const std::pair<T1, T2>& x, const std::pair<T1, T2>& y) const {
      return comp(x.first, y.first);
    };
  };

  // This is the case where the output-iterators contain nodes.
  template <typename InputIterator,
            typename DistanceValue,
            typename PairPriorityQueue,
            typename OutputIterator>
  inline
  typename boost::enable_if<
    boost::is_same<
      typename std::iterator_traits< OutputIterator >::value_type,
      typename std::iterator_traits< InputIterator >::value_type
    >,
  OutputIterator >::type copy_neighbors_from_queue(PairPriorityQueue& Q, OutputIterator result)
  {
    OutputIterator first = result;
    while( !Q.empty() )
    {
      *result = *(Q.top().second);
      Q.pop();
      ++result;
    };
    std::reverse(first, result);
    return result;
  };

};

#endif
