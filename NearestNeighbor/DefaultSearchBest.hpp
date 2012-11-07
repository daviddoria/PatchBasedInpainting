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

#ifndef DefaultSearchBest_HPP
#define DefaultSearchBest_HPP

/**
 * This functor has the same signature as other single-best-neighbor search functors,
 * but it does not actually do anything - it simply returns the first item in the container.
 * This is often used when the container is already known to be sorted by the same criterion
 * that we would have searched for.
 * If the descriptors are not all valid, you may want to use FirstValidDescriptor instead.
 */
struct DefaultSearchBest
{
  template <typename ForwardIteratorType>
  typename ForwardIteratorType::value_type operator()(ForwardIteratorType first, ForwardIteratorType last,
                                 typename ForwardIteratorType::value_type query)
  {
    return *first;
  }
};

#endif
