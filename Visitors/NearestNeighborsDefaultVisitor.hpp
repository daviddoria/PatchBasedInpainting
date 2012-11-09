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

#ifndef NearestNeighborsDefaultVisitor_HPP
#define NearestNeighborsDefaultVisitor_HPP

#include "Priority/Priority.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**

 */
struct NearestNeighborsDefaultVisitor
{
  template <typename TContainer>
  void FoundNeighbors(const TContainer& container)
  {
    // std::cout << "NearestNeighborsDefaultVisitor: Found " << container.size() << " neighbors." << std::endl;
  }

}; // NearestNeighborsDefaultVisitor

#endif
