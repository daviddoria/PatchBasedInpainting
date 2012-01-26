#ifndef InpaintingGridNoInit_HPP
#define InpaintingGridNoInit_HPP

// Custom
#include "InpaintingAlgorithm.hpp"
#include "InpaintingVisitorConcept.hpp"
#include "InpaintingVisitorWrapper.hpp"

// Boost
#include <boost/graph/detail/d_ary_heap.hpp>
#include <boost/graph/properties.hpp>

/**
 * This function template performs an in-painting of the hole-vertices of a graph.
 * This version performs no initialization meaning that it expects all the priority values,
 * position values, color-values (black for holes, white if not), as well as any other 
 * property associated to the vertices of the graph (e.g. needed by the visitor or other 
 * functors) to be already initialized to the correct values. 
 * 
 * \tparam VertexListGraph A graph type that can hold a list of vertices.
 * \tparam InpaintingVisitor A visitor type that models the InpaintingVisitorConcept which is used to inject all the custom code into this algorithm.
 * \tparam Topology A topology type that can compute a distance-value between two positions.
 * \tparam PositionMap A property-map type that can fetch the positions associated to vertices.
 * \tparam ColorMap A property-map type that can fetch the color-values associated to vertices.
 * \tparam PriorityMap A property-map type that can fetch the priority-values associated to vertices.
 * \tparam PriorityCompare A functor type that can compare priority-values (strict weak ordering).
 * \tparam NearestNeighborFinder A functor type that can find the nearest neighbor to a given vertex within a given topology.
 * \tparam PatchInpainter A functor type that traverse the holes in a patch and apply a visitor painting on them.
 * \param g A graph that holds all the vertices.
 * \param vis A visitor to inject all the custom code into this algorithm.
 * \param space A topology to compute a distance-value between two positions.
 * \param position A property-map to fetch the positions associated to vertices.
 * \param color A property-map to fetch the color-values associated to vertices.
 * \param priority A property-map to fetch the priority-values associated to vertices.
 * \param compare_priority A functor to compare priority-values (strict weak ordering).
 * \param find_inpainting_source A functor to find the nearest neighbor to a given vertex within a given topology.
 * \param inpaint_patch A functor to traverse the holes in a patch and apply a visitor's painting on them.
 */
template <typename VertexListGraph, typename InpaintingVisitor,
          typename Topology, typename PositionMap,
          typename ColorMap, typename PriorityMap, 
          typename PriorityCompare, 
          typename NearestNeighborFinder, 
          typename PatchInpainter>
inline
void inpainting_grid_no_init(VertexListGraph& g, InpaintingVisitor vis,
                        const Topology& space, PositionMap position,
                        ColorMap color, PriorityMap priority, 
                        PriorityCompare compare_priority,
                        NearestNeighborFinder find_inpainting_source, 
                        PatchInpainter inpaint_patch) {
  
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<InpaintingVisitor,VertexListGraph>));
  
  typedef typename boost::graph_traits<VertexListGraph>::vertex_descriptor Vertex;
  typedef typename boost::property_traits<ColorMap>::value_type ColorValue;
  typedef boost::color_traits<ColorValue> Color;
  typedef typename boost::property_traits<PriorityMap>::value_type PriorityValue;
  
  typedef typename boost::property_map<VertexListGraph, boost::vertex_index_t>::const_type GridIndexMapType;
  typedef boost::vector_property_map<std::size_t, GridIndexMapType> IndexInHeapMap;
  IndexInHeapMap index_in_heap;

  typedef boost::d_ary_heap_indirect<Vertex, 4, IndexInHeapMap, PriorityMap, PriorityCompare> MutableQueue;
  MutableQueue boundaryNodeQueue(priority, index_in_heap, compare_priority); //priority queue

  // TODO: Is it ok to add the vertex to the index_in_heap at the same time as pushing it onto the queue (like is done below)?
  // add all the black vertices (holes) to the priority-queue:
  for (typename boost::graph_traits<VertexListGraph>::vertices_size_type v_index = 0; v_index < num_vertices(g); ++v_index) 
    {
    Vertex v = vertex(v_index, g);
    if( get(color, v) == Color::black() )
      {
      put(index_in_heap, v, static_cast<std::size_t>(-1));

      boundaryNodeQueue.push(v);  // the priority-queue will automatically get the priority and put the vertex in the right order (hence the "indirect")
      }
    }

  // start the in-painting loop:
  inpainting_loop(g, inpainting_visitor_wrapper<InpaintingVisitor,ColorMap>(vis,color),
                  space, position, color, boundaryNodeQueue, find_inpainting_source, inpaint_patch);
  
};

#endif
