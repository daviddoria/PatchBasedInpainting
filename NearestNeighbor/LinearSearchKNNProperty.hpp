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
  * \tparam DistanceValueType The value-type for the distance measures.
  * \tparam DistanceFunctionType The functor type to compute the distance measure.
  * \tparam CompareFunctionType The functor type that can compare two distance measures (strict weak-ordering).
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

  /** Set the number of nearest neighbors to return. */
  void SetK(const unsigned int k)
  {
    this->K = k;
  }

  /** Get the number of nearest neighbors to return. */
  unsigned int GetK() const
  {
    return this->K;
  }

  //   template <typename ForwardIteratorType, typename OutputContainerType>
  //   void operator()(ForwardIteratorType first, ForwardIteratorType last,
  //                   typename ForwardIteratorType::value_type queryNode, OutputContainerType& output)
  /**
    * \tparam ForwardIterator The forward-iterator type.
    * \tparam OutputContainer The container type which can contain the list of nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search (usually container.end() ).
    * \param output The container that will have the sorted list of elements with the smallest distance.
    * \param compare A callable object that returns true if the first element is the preferred one (less-than) of the two.
    * \param max_neighbors The maximum number of elements of smallest distance to output in the sorted list.
    */
  template <typename ForwardIteratorType, typename OutputIteratorType>
  inline
  OutputIteratorType operator()(ForwardIteratorType first,
                                ForwardIteratorType last,
                                typename ForwardIteratorType::value_type queryNode,
                                OutputIteratorType output_first,
                                DistanceValueType radius = std::numeric_limits<DistanceValueType>::infinity())
  {
    // Nothing to do if the input range is empty
    if(first == last)
    {
      return output_first;
    }

    typedef std::pair<DistanceValueType, ForwardIteratorType> PairType;
    typedef std::priority_queue< PairType,
                          std::vector<PairType>,
                          compare_pair_first<DistanceValueType, ForwardIteratorType, CompareFunctionType> > PriorityQueueType;

    PriorityQueueType output_queue = PriorityQueueType(compare_pair_first<DistanceValueType, ForwardIteratorType,
                                                       CompareFunctionType>(this->CompareFunction));

    for(; first != last; ++first)
    {
      DistanceValueType d = DistanceFunction(get(PropertyMap, *first), get(PropertyMap, queryNode));
      if(!this->CompareFunction(d, radius))
      {
        continue;
      }
      output_queue.push(PairType(d, first));
      while(output_queue.size() > this->K)
      {
        output_queue.pop();
      }
      radius = output_queue.top().first;
    };

    return copy_neighbors_from_queue<PriorityQueueType, OutputIteratorType>(output_queue, output_first);
  };

  template <typename T1, typename T2, typename Compare>
  struct compare_pair_first : std::binary_function< std::pair<T1, T2>, std::pair<T1, T2>, bool>
  {
    Compare comp;
    compare_pair_first(const Compare& aComp = Compare()) : comp(aComp) { };
    bool operator()(const std::pair<T1, T2>& x, const std::pair<T1, T2>& y) const
    {
      return comp(x.first, y.first);
    };
  };

  /** This is the case where the output iterators contain nodes.
    * 'result' does not have to be passed by reference because it is an iterator.
    * The return type if this function is enabled is OutputIteratorType.
    */
  template <typename PairPriorityQueueType,
            typename OutputIteratorType>
  inline
  typename boost::enable_if<
    boost::is_same<
      typename std::iterator_traits<typename PairPriorityQueueType::value_type::second_type >::value_type,
      typename std::iterator_traits< OutputIteratorType >::value_type
    >,
  OutputIteratorType >::type copy_neighbors_from_queue(PairPriorityQueueType& Q, OutputIteratorType result)
  {
    OutputIteratorType first = result;
    while( !Q.empty() )
    {
      *result = *(Q.top().second);
      Q.pop();
      ++result;
    };
    std::reverse(first, result);
    return result;
  };

    /** This is a dummy function to throw a more sane error if the SFINAE function does not match. */
//   template <typename PairPriorityQueueType,
//             typename OutputIteratorType>
//   inline
//   OutputIteratorType copy_neighbors_from_queue(PairPriorityQueueType& Q, OutputIteratorType result)
//   {
//     // #error The only SFINAE overload did not match!
//     return OutputIteratorType();
//   };
  
};

#endif
