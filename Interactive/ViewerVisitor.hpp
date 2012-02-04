#ifndef ViewerVisitor_HPP
#define ViewerVisitor_HPP

/**
 * This is a visitor that complies with the InpaintingVisitorConcept and forwards
 * its events to another visitor. The only addition is that it displays the output
 * at each iteration.
 */
template <typename InternalVisitorType>
class ViewerVisitor
{
private:
  InternalVisitorType InternalVisitor;

public:

  ViewerVisitor(InternalVisitorType internalVisitor) : InternalVisitor(internalVisitor) {}

  template <typename VertexType, typename Graph>
  void initialize_vertex(VertexType v, Graph& g) const
  {
    InternalVisitor->initialize_vertex(v, g);
  };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType v, Graph& g) const 
  {
    InternalVisitor->discover_vertex(v, g);
  };

  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType a, VertexType b, Graph& g) const 
  {
    InternalVisitor->vertex_match_made(a, b, g);
  };

  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType a, VertexType b, Graph& g) const 
  {
    InternalVisitor->paint_vertex(a, b, g);
  };

  template <typename VertexType, typename Graph>
  bool accept_painted_vertex(VertexType v, Graph& g) const 
  {
    return InternalVisitor->accept_painted_vertex(v, g);
  };

  template <typename VertexType, typename Graph>
  void finish_vertex(VertexType v, Graph& g) const 
  { 
    InternalVisitor->finish_vertex(v, g);
  };

};

#endif
