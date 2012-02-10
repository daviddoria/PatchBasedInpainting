#ifndef CompositeInpaintingVisitor_HPP
#define CompositeInpaintingVisitor_HPP

#include "InpaintingVisitorParent.h"

#include <boost/graph/graph_traits.hpp>

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TGraph>
struct CompositeInpaintingVisitor 
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->initialize_vertex(v, g);
      }
  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const 
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->discover_vertex(v, g);
      }
  };

  void vertex_match_made(VertexDescriptorType a, VertexDescriptorType b, TGraph& g) const 
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->vertex_match_made(a, b, g);
      }
  };

  void paint_vertex(VertexDescriptorType a, VertexDescriptorType b, TGraph& g) const 
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->paint_vertex(a, b, g);
      }
  };

  bool accept_painted_vertex(VertexDescriptorType v, TGraph& g) const 
  {
    bool acceptAll = true;
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      bool accept = Visitors[visitorId]->accept_painted_vertex(v, g);
      acceptAll = acceptAll && accept;
      }
    return acceptAll;
  };

  void finish_vertex(VertexDescriptorType v, TGraph& g) const 
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->finish_vertex(v, g);
      }
  };

  void AddVisitor(InpaintingVisitorParent<TGraph>* vis)
  {
    this->Visitors.push_back(vis);
  }

private:
  std::vector<InpaintingVisitorParent<TGraph>*> Visitors;
};

#endif
