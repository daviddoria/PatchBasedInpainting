#ifndef DescriptorVisitorConcept_HPP
#define DescriptorVisitorConcept_HPP

#include <boost/graph/graph_concepts.hpp>
#include <boost/concept_check.hpp>

/**
 * This concept-check class defines the functions that a descriptor visitor must have in order to be 
 * used by the inpainting visitors.
 * 
 * Valid Expressions:
 * 
 *  vis.initialize_vertex(u, g);  Called on all vertices during the initialization phase.
 * 
 *  vis.discover_vertex(u, g);  Called when a live vertex is taken out of the priority-queue.
 * 
 * \tparam TDescriptorVisitor The visitor whose compliance to this concept is to be assessed.
 * \tparam TVertexListGraph The type of the graph on which the visitor will be applied.
 */
template <typename TDescriptorVisitor, typename TVertexListGraph>
struct DescriptorVisitorConcept 
{
  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor Vertex;
  Vertex u;
  TVertexListGraph g;
  TDescriptorVisitor vis;

  BOOST_CONCEPT_USAGE(DescriptorVisitorConcept) 
  {
    vis.initialize_vertex(u);// Function called on all vertices during the initialization phase.
    vis.discover_vertex(u);  // Function called when a live vertex is taken out of the priority-queue.
  };

};

#endif
