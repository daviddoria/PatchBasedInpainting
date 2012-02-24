#ifndef CompositeAcceptanceVisitor_HPP
#define CompositeAcceptanceVisitor_HPP

#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

#include <boost/graph/graph_traits.hpp>

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TGraph>
struct CompositeAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& energy) const
  {
    bool acceptOverride = false;

    // If any of these visitors passes, automatically accept the pair.
    for(unsigned int visitorId = 0; visitorId < OverrideVisitors.size(); ++visitorId)
      {
      float energy;
      acceptOverride |= OverrideVisitors[visitorId]->AcceptMatch(target, source, energy);
      }

    if(acceptOverride)
    {
      std::cout << "Accepted override!" << std::endl;
      return acceptOverride;
    }

    // If we get to here, all of the following visitors must pass.
    // (we compute them all instead of returning immediate if one fails so we can inspect the outputs)
    bool acceptAll = true;
    for(unsigned int visitorId = 0; visitorId < RequiredPassVisitors.size(); ++visitorId)
      {
      float energy;
      bool accept = RequiredPassVisitors[visitorId]->AcceptMatch(target, source, energy);
      acceptAll = acceptAll && accept;
      }
    return acceptAll;
  };

  void AddOverrideVisitor(AcceptanceVisitorParent<TGraph>* vis)
  {
    this->OverrideVisitors.push_back(vis);
  }

  void AddRequiredPassVisitor(AcceptanceVisitorParent<TGraph>* vis)
  {
    this->RequiredPassVisitors.push_back(vis);
  }

private:
  std::vector<AcceptanceVisitorParent<TGraph>*> RequiredPassVisitors;
  std::vector<AcceptanceVisitorParent<TGraph>*> OverrideVisitors;
};

#endif
