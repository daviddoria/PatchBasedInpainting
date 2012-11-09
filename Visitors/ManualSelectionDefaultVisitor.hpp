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

#ifndef ManualSelectionDefaultVisitor_HPP
#define ManualSelectionDefaultVisitor_HPP

// Custom
#include "Node.h"

// STL
#include <iostream>
#include <vector>

/**

 */
template <typename TVertexDescriptor>
class ManualSelectionDefaultVisitor
{
public:

  // ManualSelectionDefaultVisitor(){}

  /** Return the best source node for a specified target node. This default implementation
   *  simply returns the first node from the potenial set. */
  template <typename TForwardIterator>
  TVertexDescriptor select(const TVertexDescriptor& targetNode,
                           TForwardIterator possibleNodesBegin, TForwardIterator possibleNodesEnd)
  {
    return *possibleNodesBegin;
  }

}; // ManualSelectionDefaultVisitor

#endif
