#ifndef ImagePatchInpaintingVisitor_HPP
#define ImagePatchInpaintingVisitor_HPP

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It creates
 * and differences ImagePatch objects at each pixel.
 */
struct ImagePatch_inpainting_visitor 
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
