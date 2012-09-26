#ifndef LinearSearchKNN_HPP
#define LinearSearchKNN_HPP

// STL
#include <limits> // for infinity()
#include <algorithm> // for lower_bound()

// Custom
#include "Utilities/Utilities.hpp"

/**
  * This function template is similar to std::min_element but can be used when the comparison
  * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the
  * the elements in the range [first,last) with the "smallest" distances (of course, both the
  * distance metric and comparison can be overriden to perform something other than the canonical
  * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
  * This function will fill the output container with a number of nearest-neighbors.
  * \tparam TForwardIterator The forward-iterator type.
  * \tparam TOutputContainer The container type which can contain the list of nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
  * \tparam TDistanceFunction The functor type to compute the distance measure.
  * \tparam TDistanceValue The value-type for the distance measures.
  * \tparam TCompareFunction The functor type that can compare two distance measures (strict weak-ordering).
  */
template <typename TForwardIterator,
          typename TOutputContainer,
          typename TDistanceFunction,
          typename TDistanceValue = float,
          typename TCompareFunction = std::less<TDistanceValue> >
struct LinearSearchKNN
{
  typedef TOutputContainer OutputContainerType;

  /** The function to compute the distance between two elements.*/
  TDistanceFunction DistanceFunction;

  /** The function to compare two elements (used to sort).*/
  TCompareFunction CompareFunction;

  /** Sometimes we don't want to search exhaustively - this lets us take steps through the container
   * of a specified StrideLength rather than comparing to every element in the container.*/
  unsigned int StrideLength;

  LinearSearchKNN(TDistanceFunction distanceFunction = TDistanceFunction(), const unsigned int k = 1000,
                  const unsigned int strideLength = 1) : DistanceFunction(distanceFunction), K(k), StrideLength(strideLength)
  {
  }

  /** Set the number of nearest neighbors to return */
  void SetK(const unsigned int k)
  {
    this->K = k;
  }

  /** Get the number of nearest neighbors to return */
  unsigned int GetK() const
  {
    return this->K;
  }

  /** The function that actually performs the search
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param output The container that will have the sorted list of elements with the smallest distance.
    */
  void operator()(TForwardIterator first, TForwardIterator last, OutputContainerType& output)
  {
    output.clear();

    // There is nothing to do if the specified range of the input container is empty.
    if(first == last)
    {
      return;
    }

    std::vector<TDistanceValue> output_dist;

    // Iterate through every input element
    //for(; first != last; ++first)

    // Advance through the input elements by StrideLength until we pass the end of the input range.
    while(try_advance(first, last, this->StrideLength))
    {
      TDistanceValue d = DistanceFunction(*first);
      if(!CompareFunction(d, std::numeric_limits<TDistanceValue>::infinity()))
      {
        continue;
      }
      typename std::vector<TDistanceValue>::iterator it_lo = std::lower_bound(output_dist.begin(),
                                                                              output_dist.end(), d, CompareFunction);
      if((it_lo != output_dist.end()) || (output_dist.size() < this->K))
      {
        output_dist.insert(it_lo, d);
        typename OutputContainerType::iterator itv = output.begin();
        for(typename std::vector<TDistanceValue>::iterator it = output_dist.begin();
            (itv != output.end()) && (it != it_lo); ++itv,++it) ;
        output.insert(itv, *first);
        if(output.size() > this->K)
        {
          output.pop_back();
          output_dist.pop_back();
        }
      }
    }
  }

private:
  /** The number of nearest neighbors to return */
  unsigned int K;
};

#endif
