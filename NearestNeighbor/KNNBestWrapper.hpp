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

#ifndef KNNBestWrapper_HPP
#define KNNBestWrapper_HPP

// STL
#include <memory>

/**
  * This class uses a KNN finder followed by a Best finder on the output of
  * the KNNFinder.
  * \tparam KNNFinderType A KNN searcher
  * \tparam BestFinderType A Best search
  */
template <typename KNNFinderType, typename BestFinderType>
class KNNBestWrapper
{
private:
  std::shared_ptr<KNNFinderType> KNNFinder;
  std::shared_ptr<BestFinderType> BestFinder;

public:
  KNNBestWrapper(std::shared_ptr<KNNFinderType> knnFinder, std::shared_ptr<BestFinderType> bestFinder) :
    KNNFinder(knnFinder), BestFinder(bestFinder)
  {
  }

  /**
    * \tparam TIterator The forward-iterator type.
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search (usually container.end() ).
    * \param queryNode The item to compare the items in the container against.
    * \param outputFirst An iterator to the beginning of the output container that will store the K nearest neighbors.
    */
  template <typename TIterator>
  inline
  typename TIterator::value_type operator()(const TIterator first,
                                            const TIterator last,
                                            typename TIterator::value_type queryNode)
  {
    // Allocate a vector to get the results of the KNN search
    std::vector<typename TIterator::value_type> knnContainer(this->KNNFinder->GetK());

    // Perform the KNN search
    (*this->KNNFinder)(first, last, queryNode, knnContainer.begin());

    // Perform the best search
    typename TIterator::value_type bestVertex = (*this->BestFinder)(knnContainer.begin(),
                                                                    knnContainer.end(), queryNode);
    return bestVertex;
  }
  
};

#endif
