#ifndef CompositeInpaintingVisitor_HPP
#define CompositeInpaintingVisitor_HPP

#include "InpaintingVisitorParent.h"

/**
 * This is a composite visitor type that complies with the InpaintingVisitorConcept and forwards
 * all calls to all of its internal visitors.
 */
template <typename TGraph>
struct CompositeInpaintingVisitor 
{
  template <typename VertexType, typename Graph>
  void initialize_vertex(VertexType v, Graph& g) const
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->initialize_vertex(v, g);
      }
  };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType v, Graph& g) const 
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->discover_vertex(v, g);
      }
  };

  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType a, VertexType b, Graph& g) const 
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->vertex_match_made(a, b, g);
      }
  };

  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType a, VertexType b, Graph& g) const 
  { 
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->paint_vertex(a, b, g);
      }
  };

  template <typename VertexType, typename Graph>
  bool accept_painted_vertex(VertexType v, Graph& g) const 
  {
    bool accept;
    for(unsigned int visitorId = 0; visitorId < Visitors.size(); ++visitorId)
      {
      Visitors[visitorId]->accept_painted_vertex(v, g);
      }
    return true; 
  };

  template <typename VertexType, typename Graph>
  void finish_vertex(VertexType v, Graph& g) const 
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
