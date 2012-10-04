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

#ifndef LinearSearchKNNPropertyCombine_HPP
#define LinearSearchKNNPropertyCombine_HPP

/**
  * This class simply runs two KNNProperty searchers and combines their results.
  */
template <typename TKNNSearcher1, typename TKNNSearcher2>
class LinearSearchKNNPropertyCombine
{
  TKNNSearcher1 KNNSearcher1;
  TKNNSearcher2 KNNSearcher2;

public:
  LinearSearchKNNPropertyCombine(TKNNSearcher1 knnSearcher1, TKNNSearcher2 knnSearcher2):
    KNNSearcher1(knnSearcher1), KNNSearcher2(knnSearcher2)
  {
  }

  /**
    * Return the sum of the number of results for both searchers.
    */
  unsigned int GetK() const
  {
    return this->KNNSearcher1.GetK() +   this->KNNSearcher2.GetK();
  }

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
                                OutputIteratorType outputFirst)
  {
    OutputIteratorType endOutput1 = KNNSearcher1(first, last, queryNode, outputFirst);
    return KNNSearcher2(first, last, queryNode, endOutput1);
  }

};

#endif
