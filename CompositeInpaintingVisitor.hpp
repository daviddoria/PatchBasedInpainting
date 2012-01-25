#ifndef CompositeInpaintingVisitor_HPP
#define CompositeInpaintingVisitor_HPP

/**
 * This is a default visitor type that complies with the InpaintingVisitorConcept and does 
 * nothing in all cases (can be used if there are no exogenous operations to do during the 
 * inpainting algorithm, which would be surprising given the nature of the algorithm).
 */
struct composite_inpainting_visitor 
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
  
private:
  std::vector<VisitorParent*> Visitors;
};

#endif
