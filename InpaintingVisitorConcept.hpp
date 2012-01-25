#ifndef InpaintingVisitorConcept_HPP
#define InpaintingVisitorConcept_HPP

#include <boost/graph/graph_concepts.hpp>

/**
 * This concept-check class defines the functions that a visitor must have in order to be 
 * used by the inpainting algorithms.
 * 
 * Required concepts:
 * 
 *  The visitor should be copy-constructible.
 * 
 * Valid Expressions:
 * 
 *  vis.initialize_vertex(u, g);  Called on all vertices during the initialization phase.
 * 
 *  vis.discover_vertex(u, g);  Called when a live vertex is taken out of the priority-queue.
 * 
 *  vis.vertex_match_made(target, source, g);  Called when a source vertex has been found that matches well to the current target-vertex (the same vertex that was just discovered).
 * 
 *  vis.paint_vertex(target, source, g);  Called to paint the value of a target vertex with the value of the source vertex.
 *  
 *  bool was_successfully_painted = vis.accept_painted_vertex(target, g);  Called to check if the in-painting of the target vertex was successful.
 * 
 *  vis.finish_vertex(u, g);  Called when a vertex has been inpainted and removed from the set of target pixels.
 * 
 * \tparam InpaintingVisitor The visitor whose compliance to this concept is to be assessed.
 * \tparam VertexListGraph The type of the graph on which the visitor will be applied.
 */
template <typename InpaintingVisitor, typename VertexListGraph>
struct InpaintingVisitorConcept {
  
  typedef typename boost::graph_traits<VertexListGraph>::vertex_descriptor Vertex;
  Vertex u;
  VertexListGraph g;
  InpaintingVisitor vis;
  
  BOOST_CONCEPT_ASSERT((boost::CopyConstructibleConcept<InpaintingVisitor>));
  
  BOOST_CONCEPT_USAGE(InpaintingVisitorConcept) {
    vis.initialize_vertex(u, g);  //function called on all vertices during the initialization phase.
    vis.discover_vertex(u, g);  //function called when a live vertex is taken out of the priority-queue.
    const Vertex& target = u;
    const Vertex& source = u;
    vis.vertex_match_made(target, source, g);  //function called when a source vertex has been found that matches well to the current target-vertex (the same vertex that was just discovered). 
    vis.paint_vertex(target, source, g);  //function called to paint the value of a target vertex with the value of the source vertex.
    bool was_successfully_painted = vis.accept_painted_vertex(target, g);  //function called to check if the in-painting of the target vertex was successful.
    vis.finish_vertex(u, g);  //function called when a vertex has been inpainted and removed from the set of target pixels.
  };
  
};

#endif
