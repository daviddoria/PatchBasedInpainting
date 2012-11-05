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

#ifndef KNNSearchAndSort_HPP
#define KNNSearchAndSort_HPP

// STL
#include <memory>

/**
  * This class searches a container for the K nearest neighbors of a query item.
  * The search is done by comparing the values in a property map associated with
  * each item in the container.
  * \tparam PropertyMapType The type of the property map containing the values to compare.
  * \tparam DistanceFunctionType The functor type to compute the distance measure between two items in the PropertyMap.
  */
template <typename SearchType, typename SortType>
class KNNSearchAndSort
{
private:
  std::shared_ptr<SearchType> Searcher;
  std::shared_ptr<SortType> Sorter;

public:
  KNNSearchAndSort(std::shared_ptr<SearchType> searcher, std::shared_ptr<SortType> sorter) :
    Searcher(searcher), Sorter(sorter)
  {
  }

  /**
    * \tparam TForwardIterator The forward-iterator type.
    * \tparam TOutputIterator The iterator type of the output container.
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search (usually container.end() ).
    * \param queryNode The item to compare the items in the container against.
    * \param outputFirst An iterator to the beginning of the output container that will store the K nearest neighbors.
    */
  template <typename TIterator, typename TOutputIterator>
  inline
  TOutputIterator operator()(const TIterator first,
                             const TIterator last,
                             typename TIterator::value_type queryNode,
                             TOutputIterator outputFirst)
  {
    // Allocate a vector to get the results of the KNN search
    std::vector<typename TIterator::value_type> knnContainer(this->Searcher->GetK());

    // Perform the KNN search
    (*this->Searcher)(first, last, queryNode, knnContainer.begin());

    // Sort the KNNs
    return (*this->Sorter)(knnContainer.begin(), knnContainer.end(), queryNode, outputFirst);
  }

  /** Get the number of elements that will be returned. */
  size_t GetK()
  {
    return this->Searcher->GetK();
  }
  
};

#endif
