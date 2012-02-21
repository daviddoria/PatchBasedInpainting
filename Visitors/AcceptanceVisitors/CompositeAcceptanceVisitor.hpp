#ifndef CompositeAcceptanceVisitor_HPP
#define CompositeAcceptanceVisitor_HPP

#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

#include <boost/graph/graph_traits.hpp>

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TGraph>
struct CompositeAcceptanceVisitor
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    bool acceptAll = true;
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      bool accept = Visitors[visitorId]->AcceptMatch(target, source);
      acceptAll = acceptAll && accept;
      }
    return acceptAll;
  };

  void AddVisitor(AcceptanceVisitorParent<TGraph>* vis)
  {
    this->Visitors.push_back(vis);
  }

private:
  std::vector<AcceptanceVisitorParent<TGraph>*> Visitors;
};

#endif
