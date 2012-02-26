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
  
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy = 0.0f) const
  {
    computedEnergy = sqrt(pow(target[0] - source[0], 2) + pow(target[1] - source[1], 2));

    if(computedEnergy < DistanceThreshold)
    {
      std::cout << "PatchDistanceAcceptanceVisitor passed with distance: " << computedEnergy << std::endl;
      return true;
    }
    std::cout << "PatchDistanceAcceptanceVisitor failed with distance: " << computedEnergy << std::endl;
    return false;
  }

};

#endif
