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

#ifndef LinearSearchBestPassThrough_HPP
#define LinearSearchBestPassThrough_HPP

class LinearSearchBestPassThrough
{
public:

  template <typename ForwardIteratorType, typename OutputContainerType>
  typename ForwardIteratorType::value_type operator()(ForwardIteratorType first, ForwardIteratorType last,
                                            typename ForwardIteratorType::value_type queryNode, OutputContainerType& output)
  {
    output.clear();
    std::copy(first, last, output.begin());
  }

};

#endif
