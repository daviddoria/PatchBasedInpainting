#ifndef DefaultInpaintingVisitor_HPP
#define DefaultInpaintingVisitor_HPP

/**
 * This is a default visitor type that models with the InpaintingVisitorConcept and does 
 * nothing in all cases (can be used if there are no exogenous operations to do during the 
 * inpainting algorithm, which would be surprising given the nature of the algorithm).
 */
struct DefaultInpaintingVisitor 
{
  template <typename VertexType, typename Graph>
  void initialize_vertex(VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType, VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType, VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  bool accept_painted_vertex(VertexType, Graph&) const { return true; };

  template <typename VertexType, typename Graph>
  void finish_vertex(VertexType, Graph&) const { };
};

#endif
