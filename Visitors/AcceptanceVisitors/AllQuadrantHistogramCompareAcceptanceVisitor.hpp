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

#ifndef AllQuadrantHistogramCompareAcceptanceVisitor_HPP
#define AllQuadrantHistogramCompareAcceptanceVisitor_HPP

#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

#include "Visitors/AcceptanceVisitors/QuadrantHistogramCompareAcceptanceVisitor.hpp"

#include <boost/graph/graph_traits.hpp>

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TGraph, typename TImage>
struct AllQuadrantHistogramCompareAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  // float DifferenceThreshold; // The threshold pased to this visitor is divided by 4 and passed on to each quadrant visitor

  typedef QuadrantHistogramCompareAcceptanceVisitor<TGraph, TImage> QuadrantVisitorType;
  AllQuadrantHistogramCompareAcceptanceVisitor(TImage* const image, Mask* const mask,
                                               const unsigned int halfWidth, const std::vector<float>& mins,
                                               const std::vector<float>& maxs,
                                               const float differenceThreshold = 100.0f) :
    AcceptanceVisitorParent<TGraph>("AllQuadrantHistogramCompareAcceptanceVisitor")
  {
    for(unsigned int quadrant = 0; quadrant < 4; ++quadrant)
    {
      QuadrantVisitorType* quadrantHistogramCompareAcceptanceVisitor =
          new QuadrantVisitorType(image, mask, halfWidth, quadrant, mins, maxs, differenceThreshold/4.0f);
      this->QuadrantVisitors.push_back(quadrantHistogramCompareAcceptanceVisitor);
    }
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    float energy = 0.0f;
    return AcceptMatch(target, source, energy);
  }
  
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& energy) const override
  {
    bool acceptAll = true;
    energy = 0.0f;
    for(unsigned int visitorId = 0; visitorId < this->QuadrantVisitors.size(); ++visitorId)
    {
      float quadrantEnergy = 0.0f;
      bool accept = this->QuadrantVisitors[visitorId]->AcceptMatch(target, source, quadrantEnergy);
      std::cout << "AllQuadrantHistogramCompareAcceptanceVisitor: Quadrant " << visitorId
                << " energy: " << quadrantEnergy << std::endl;
      energy += quadrantEnergy;
      acceptAll = acceptAll && accept;
    }
    return acceptAll;
  }

private:
  // Have to store pointers because there is no default constructor
  std::vector<QuadrantVisitorType*> QuadrantVisitors;
};

#endif
