#ifndef LinearSearch_HPP
#define LinearSearch_HPP

#include <algorithm>
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
template <typename ForwardIterator,
          typename DistanceFunction,
          typename DistanceValue = float,
          typename CompareFunction = std::less<DistanceValue> >
inline ForwardIterator LinearSearchBest(ForwardIterator first,
                                        ForwardIterator last,
                                        DistanceFunction distanceFunction,
                                        CompareFunction compare = CompareFunction(),
                                        DistanceValue inf = std::numeric_limits<DistanceValue>::infinity())
{
  if(first == last)
  {
    return last;
  }

  DistanceValue d_best = inf;
  ForwardIterator result = last;
  for(; first != last; ++first)
  {
    DistanceValue d = distanceFunction(*first);
    if(compare(d, d_best))
    {
      d_best = d;
      result = first;
    };
  };
  return result;
}

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
  * \param radius The maximum distance value for which an element qualifies to be part of the output list.
  */
template <typename ForwardIterator,
          typename OutputContainer,
          typename GetDistanceFunction,
          typename DistanceValue = float,
          typename CompareFunction = std::less<DistanceValue> >
inline void LinearSearchKNN(ForwardIterator first,
                            ForwardIterator last,
                            OutputContainer& output,
                            GetDistanceFunction distance,
                            unsigned int max_neighbors = 1,
                            CompareFunction compare = CompareFunction(),
                            DistanceValue radius = std::numeric_limits<DistanceValue>::infinity())
{
  output.clear();
  if(first == last)
  {
    return;
  }

  std::vector<DistanceValue> output_dist;
  for(; first != last; ++first)
  {
    DistanceValue d = distance(*first);
    if(!compare(d, radius))
    {
      continue;
    }
    typename std::vector<DistanceValue>::iterator it_lo = std::lower_bound(output_dist.begin(),output_dist.end(),d,compare);
    if((it_lo != output_dist.end()) || (output_dist.size() < max_neighbors))
    {
      output_dist.insert(it_lo, d);
      typename OutputContainer::iterator itv = output.begin();
      for(typename std::vector<DistanceValue>::iterator it = output_dist.begin();
          (itv != output.end()) && (it != it_lo); ++itv,++it) ;
      output.insert(itv, *first);
      if(output.size() > max_neighbors)
      {
        output.pop_back();
        output_dist.pop_back();
      };
    };
  };
};

#endif
