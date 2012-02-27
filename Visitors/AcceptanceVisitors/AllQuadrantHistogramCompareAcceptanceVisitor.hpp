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

  // float DifferenceThreshold;

  typedef QuadrantHistogramCompareAcceptanceVisitor<TGraph, TImage> QuadrantVisitorType;
  AllQuadrantHistogramCompareAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, const float differenceThreshold = 100.0f) :
  AcceptanceVisitorParent<TGraph>("AllQuadrantHistogramCompareAcceptanceVisitor")
  {
    for(unsigned int quadrant = 0; quadrant < 4; ++quadrant)
    {
      QuadrantVisitorType* quadrantHistogramCompareAcceptanceVisitor = new QuadrantVisitorType(image, mask, halfWidth, quadrant, differenceThreshold/4.0f);
      QuadrantVisitors.push_back(quadrantHistogramCompareAcceptanceVisitor);
    }
  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    float energy = 0.0f;
    return AcceptMatch(target, source, energy);
  }
  
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& energy) const
  {
    bool acceptAll = true;
    energy = 0.0f;
    for(unsigned int visitorId = 0; visitorId < QuadrantVisitors.size(); ++visitorId)
      {
      float quadrantEnergy = 0.0f;
      bool accept = QuadrantVisitors[visitorId]->AcceptMatch(target, source, quadrantEnergy);
      std::cout << "AllQuadrantHistogramCompareAcceptanceVisitor: Quadrant " << visitorId << " energy: " << quadrantEnergy << std::endl;
      energy += quadrantEnergy;
      acceptAll = acceptAll && accept;
      }
    return acceptAll;
  };

private:
  // Have to store pointers because there is no default constructor
  std::vector<QuadrantVisitorType*> QuadrantVisitors;
};

#endif
