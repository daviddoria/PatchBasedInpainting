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

#ifndef FirstValidDescriptor_HPP
#define FirstValidDescriptor_HPP

#include "PixelDescriptors/PixelDescriptor.h"

#include <boost/property_map/property_map.hpp>

/** This class is passed a range of nodes and a query node (the query node is not used, it is just there
  * to conform to the API of other nearest neighbor searchers. It simply returns the first source node
  * in the range.
  */
template <typename DescriptorMapType>
struct FirstValidDescriptor
{
  DescriptorMapType DescriptorMap;

  FirstValidDescriptor(DescriptorMapType descriptorMap) : DescriptorMap(descriptorMap)
  {
  }

  template <typename TForwardIterator>
  typename TForwardIterator::value_type operator()(TForwardIterator first, TForwardIterator last,
                                                   typename TForwardIterator::value_type query)
  {
    for(TForwardIterator iter = first; iter != last; ++iter)
    {
      if(get(DescriptorMap, *iter).GetStatus() == PixelDescriptor::SOURCE_NODE)
      {
        return *iter;
      }
    };
    throw std::runtime_error("FirstValidDescriptor: There were no valid descriptors!");
  }
};

#endif
