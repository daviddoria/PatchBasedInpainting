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

#ifndef InpaintingPrecomputedAlgorithm_hpp
#define InpaintingPrecomputedAlgorithm_hpp

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

// Custom
#include "Helpers/BoostHelpers.h"

template <typename TNodePairQueue, typename TVisitor, typename TPatchInpainter>
inline
void InpaintingPrecomputedAlgorithm(TNodePairQueue& nodeQueue, TVisitor vis,
                        TPatchInpainter inpaint_patch)
{
  typedef typename TNodePairQueue::value_type NodePairType;
  while(!nodeQueue.empty())
  {
    NodePairType nodePair = nodeQueue.front();
    typename NodePairType::first_type targetNode = nodePair.first;
    typename NodePairType::second_type sourceNode = nodePair.second;

    std::cout << "Filling target node: ";
    Helpers::OutputNode(targetNode);

    std::cout << "with source node: ";
    Helpers::OutputNode(sourceNode);
    
    inpaint_patch(targetNode, sourceNode);

    vis.FinishVertex(targetNode, sourceNode);

    nodeQueue.pop();
  } // end main iteration loop

  vis.InpaintingComplete();

};

#endif
