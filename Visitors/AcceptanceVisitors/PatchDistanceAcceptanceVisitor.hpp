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

#ifndef PatchDistanceAcceptanceVisitor_HPP
#define PatchDistanceAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

/**

 */
template <typename TGraph>
struct PatchDistanceAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  float DistanceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  PatchDistanceAcceptanceVisitor(const float distanceThreshold = 100) :
  DistanceThreshold(distanceThreshold)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    float computedEnergy = 0.0f;
    return AcceptMatch(target, source, computedEnergy);
  }
  
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy = 0.0f) const override
  {
    computedEnergy = sqrt(pow(target[0] - source[0], 2) + pow(target[1] - source[1], 2));

    if(computedEnergy < this->DistanceThreshold)
    {
      std::cout << "PatchDistanceAcceptanceVisitor passed with distance: " << computedEnergy << std::endl;
      return true;
    }
    std::cout << "PatchDistanceAcceptanceVisitor failed with distance: " << computedEnergy << std::endl;
    return false;
  }

};

#endif
