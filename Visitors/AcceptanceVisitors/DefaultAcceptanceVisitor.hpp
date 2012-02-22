#ifndef DefaultAcceptanceVisitor_HPP
#define DefaultAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

/**

 */
template <typename TGraph>
struct DefaultAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    return true;
  };

};

#endif
