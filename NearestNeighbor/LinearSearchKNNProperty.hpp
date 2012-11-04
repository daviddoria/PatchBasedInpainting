/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

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
  * This class searches a container for the K nearest neighbors of a query item.
  * The search is done by comparing the values in a property map associated with
  * each item in the container.
  * \tparam PropertyMapType The type of the property map containing the values to compare.
  * \tparam DistanceFunctionType The functor type to compute the distance measure between two items in the PropertyMap.
  */
template <typename PropertyMapType,
          typename DistanceFunctionType>
class LinearSearchKNNProperty
{
  typedef float DistanceValueType;

  std::shared_ptr<PropertyMapType> PropertyMap;
  unsigned int K;
  DistanceFunctionType DistanceFunction;

public:
  LinearSearchKNNProperty(std::shared_ptr<PropertyMapType> propertyMap, const unsigned int k = 1000,
                          DistanceFunctionType distanceFunction = DistanceFunctionType()) :
    PropertyMap(propertyMap), K(k), DistanceFunction(distanceFunction)
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

  /**
    * \tparam TForwardIterator The forward-iterator type.
    * \tparam TOutputIterator The iterator type of the output container.
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search (usually container.end() ).
    * \param queryNode The item to compare the items in the container against.
    * \param outputFirst An iterator to the beginning of the output container that will store the K nearest neighbors.
    */
  template <typename TForwardIteratorType, typename TOutputIterator>
  inline
  TOutputIterator operator()(TForwardIteratorType first,
                             TForwardIteratorType last,
                             typename TForwardIteratorType::value_type queryNode,
                             TOutputIterator outputFirst)
  {
    // Nothing to do if the input range is empty
    if(first == last)
    {
      return outputFirst;
    }

    // Create a type to associate a distance with its iterator
    typedef std::pair<DistanceValueType, TForwardIteratorType> PairType;

    // Use a priority queue to keep the items (pairs of distances and their corresponding iterators) sorted
    typedef std::priority_queue< PairType,
        std::vector<PairType>,
        compare_pair_first<DistanceValueType, TForwardIteratorType,
        std::greater<DistanceValueType> > > PriorityQueueType;

    PriorityQueueType outputQueue;

    // Get the query object
    typename PropertyMapType::value_type queryPatch = get(*(this->PropertyMap), queryNode);

    // Get the offsets to compare
    typedef std::vector<itk::Offset<2> > OffsetVectorType;
    const OffsetVectorType* validOffsets = queryPatch.GetValidOffsetsAddress();

    // Create a container to store the pixels that we will compare
    typedef std::vector<typename PropertyMapType::value_type::ImageType::PixelType> PixelVector;
    PixelVector targetPixels(validOffsets->size());

    // Extract the pixel values that we want to compare
    for(OffsetVectorType::const_iterator offsetIterator = validOffsets->begin();
        offsetIterator < validOffsets->end(); ++offsetIterator)
    {
      itk::Offset<2> currentOffset = *offsetIterator;

      targetPixels[offsetIterator - validOffsets->begin()] = queryPatch.GetImage()->GetPixel(queryPatch.GetCorner() + currentOffset);
    }

    // The queue stores the items in descending score order.
    #pragma omp parallel for
//    for(ForwardIteratorType current = first; current != last; ++current) // OpenMP 3 doesn't allow != in the loop ending condition
    for(TForwardIteratorType current = first; current < last; ++current)
    {
      typename PropertyMapType::value_type currentPatch = get(*(this->PropertyMap), *current);
      // Argument order is (source, target) ("query node" is the same as "target node")
      DistanceValueType d = this->DistanceFunction(currentPatch, queryPatch, targetPixels);

      #pragma omp critical // There are weird crashes without this guard (concurrent access?)
      outputQueue.push(PairType(d, current));
    }

//    std::cout << "There are " << outputQueue.size() << " items in the queue." << std::endl;

    // Keep only the best K matches
    Helpers::KeepTopN(outputQueue, this->K);

//    std::cout << "There are " << outputQueue.size() << " items in the queue." << std::endl;

    // Check if any of the top K matches are infinity (indicating they were invalid for some reason)
    {
      auto hasInvalidValue = [] (PriorityQueueType q)
      {
        while(!q.empty())
        {
          if(q.top().first == std::numeric_limits<float>::infinity() ||
             q.top().first == std::numeric_limits<float>::max())
          {
            return true;
          }
          q.pop();
        }

        return false;
      };

      assert(!hasInvalidValue(outputQueue));
      if(hasInvalidValue(outputQueue))
      {
        std::stringstream ss;
        ss << "LinearSearchKNNProperty::operator(): One of the matches had an invalid score!";
        throw std::runtime_error(ss.str());
      }
    }

    if(outputQueue.size() < this->K)
    {
      std::stringstream ss;
      ss << "Requested " << this->K << " items but only found " << outputQueue.size();
      throw std::runtime_error(ss.str());
    }

    std::cout << "Best patch score is: " << outputQueue.top().first << std::endl;

    // Copy the best matches from the queue into the output
    TOutputIterator currentOutputIterator = outputFirst;
    while( !outputQueue.empty() )
    {
      *currentOutputIterator = *(outputQueue.top().second);
      outputQueue.pop();
      ++currentOutputIterator;
    }

    return currentOutputIterator;
  } // end operator()

  template <typename T1, typename T2, typename Compare>
  struct compare_pair_first : std::binary_function< std::pair<T1, T2>, std::pair<T1, T2>, bool>
  {
    Compare comp;
    compare_pair_first(const Compare& aComp = Compare()) : comp(aComp) { }
    bool operator()(const std::pair<T1, T2>& x, const std::pair<T1, T2>& y) const
    {
      return comp(x.first, y.first);
    }
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
  }
  
};

#endif
