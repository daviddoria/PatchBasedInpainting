/**
 * \file InpaintingAlgorithmBasic.hpp
 *
 * This library implements some core elements of an in-painting algorithm. Most of the 
 * application-specific code is relegated to the visitors used by the algorithm, and by 
 * the topology according to which the target patch is compared to potential source 
 * patches.
 * 
 * \author Sven Mikael Persson <mikael.s.persson@gmail.com>
 * \date January 2012
 */

/*
 *    Copyright 2011 Sven Mikael Persson
 *
 *    THIS SOFTWARE IS DISTRIBUTED UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE v3 (GPLv3).
 *
 *    This file is part of ReaK.
 *
 *    ReaK is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    ReaK is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with ReaK (as LICENSE in the root folder).  
 *    If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef INPAINTING_ALGORITHM_BASIC_H
#define INPAINTING_ALGORITHM_BASIC_H



#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>
#include <boost/property_map/property_map.hpp>



namespace boost {

  enum vertex_hole_priority_t { vertex_hole_priority };
  
  BOOST_INSTALL_PROPERTY(vertex, hole_priority);

};


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

/**
 * This is a default visitor type that complies with the InpaintingVisitorConcept and does 
 * nothing in all cases (can be used if there are no exogenous operations to do during the 
 * inpainting algorithm, which would be surprising given the nature of the algorithm).
 */
struct default_inpainting_visitor {
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


namespace detail {

  template <typename InpaintingVisitor, typename ColorMap>
  struct inpainting_visitor_wrapper {
    typedef typename boost::property_traits<ColorMap>::value_type ColorValue;
    typedef boost::color_traits<ColorValue> Color;
    InpaintingVisitor vis;
    ColorMap color;
    inpainting_visitor_wrapper(InpaintingVisitor aVis, ColorMap aColor) : vis(aVis), color(aColor) { };
    
    template <typename VertexType, typename Graph>
    void discover_vertex(VertexType u, Graph& g) const { 
      vis.discover_vertex(u, g);  // forward to the user's visitor.
    };
    template <typename VertexType, typename Graph>
    void vertex_match_made(VertexType target, VertexType source, Graph& g) const {
      vis.vertex_match_made(target, source, g);  //forward to user's visitor.
    };
    template <typename VertexType, typename Graph>
    void paint_vertex(VertexType target, VertexType source, Graph& g) const { 
      vis.paint_vertex(target, source, g);  //forward to user's visitor.
      //check if the painting was acceptable:
      if( vis.accept_painted_vertex(target, g) ) {
	//if yes, then this vertex is no longer a hole, so, color it white and finish it.
	using boost::put;
	put(color, target, Color::white());
	vis.finish_vertex(target, g);  //tell the user's visitor that the target vertex has be successfully painted.
      };
    };
    
  };
  

  template <typename VertexListGraph, typename WrappedInpaintingVisitor,
            typename Topology, typename PositionMap,
            typename ColorMap, typename PriorityQueue, 
	    typename NearestNeighborFinder, 
	    typename PatchInpainter>
  inline
  void inpainting_loop(VertexListGraph& g, WrappedInpaintingVisitor vis,
		       const Topology& space, PositionMap position,
                       ColorMap color, PriorityQueue& Q,
		       NearestNeighborFinder find_inpainting_source, 
		       PatchInpainter inpaint_patch) {
    
    using namespace boost;
    typedef typename graph_traits<VertexListGraph>::vertex_descriptor Vertex;
    typedef typename graph_traits<VertexListGraph>::edge_descriptor Edge;
    typedef typename property_traits<ColorMap>::value_type ColorValue;
    typedef color_traits<ColorValue> Color;
    
    // When this function is called, the priority-queue should already be filled 
    // with all the hole-vertices (which should also have "Color::black()" color value).
    // So, the only thing this function does is run the inpainting loop, but all the 
    // actual code is externalized in the functors and visitors (vis, find_inpainting_source, inpaint_patches, etc.).

    while(true) {
      // find the next target to in-paint:
      Vertex target_patch_center;
      do {
	if( Q.empty() )
	  return;  //terminate if the queue is empty.
	target_patch_center = Q.top(); Q.pop();
      } while( get(color, target_patch_center) == Color::white() );
      
      // at this point, we must have a hole target center.
      vis.discover_vertex(target_patch_center, g);   //notify the visitor.
      
      // next, we want to find a source patch-center that matches best to our target-patch:
      //  the nearest-neighbor finder will possibly need the target-patch, the graph, the topology 
      //  and a property-map to get the "position" value ("position" is a point in the topology).
      //  Note, this is the standard form of my nearest-neighbor finders (any, linear, approximate, DVP-tree, etc.).
      Vertex source_patch_center = find_inpainting_source( target_patch_center, g, space, position );
      vis.vertex_match_made(target_patch_center, source_patch_center, g);  //notify the visitor.
      
      // finally, do the in-painting of the target patch from the source patch.
      //  the inpaint_patch functor should take care of iterating through the vertices in both 
      //  patches and call "vis.paint_vertex(target, source, g)" on the individual vertices.
      inpaint_patch(target_patch_center, source_patch_center, g, vis);
      
    };
    
  };

};


/**
 * This class template is a patch inpainter (i.e. paints the holes in one patch with the values of 
 * some given source patch). This class requires that the vertices at the center of the target 
 * patches hold a list of all the holes in the target patch (as a container of vertex descriptors).
 * Also, it requires a property mapping back and forth between the identity of a vertex (vertex-descriptor)
 * and its coordinate (which should be an arithmetic type, e.g., such as a 2D vector of integers).
 * 
 * \tparam CoordinateMap A property-map to obtain the coordinates of a given vertex.
 * \tparam HoleListMap A lvalue property-map to obtain a reference to a list (i.e. container) of neighboring holes.
 * \tparam InverseCoordMap A property-map to obtain the vertex descriptor for a given set of coordinates.
 */
template <typename CoordinateMap, typename HoleListMap, typename InverseCoordMap>
struct hole_list_patch_inpainter {
  CoordinateMap coord;   //property-map used to get the grid-coordinates of a vertex.
  HoleListMap nearby_holes;   //property-map used to get the list of nearby holes around a target center vertex.
  InverseCoordMap inv_coord;   //property-map used to get the vertex-descriptor from some given grid-coordinates
  
  
  hole_list_patch_inpainter(CoordinateMap aCoord, HoleListMap aNearbyHoles, InverseCoordMap aInvCoord) : coord(aCoord), nearby_holes(aNearbyHoles), inv_coord(aInvCoord) { };
  
  template <typename Vertex, typename Graph, typename InpaintingVisitor>
  void operator()(Vertex target, Vertex source, Graph& g, InpaintingVisitor vis) {
    typedef typename boost::property_traits<HoleListMap>::value_type HoleList;
    typedef typename boost::property_traits<CoordinateMap>::value_type CoordType;
    
    BOOST_CONCEPT_ASSERT((boost::LvaluePropertyMapConcept<HoleListMap, Vertex>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<CoordinateMap, Vertex>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<InverseCoordMap, CoordType>));
    using boost::get; 
    
    HoleList& hole_list = nearby_holes[target];
    CoordType target_center = get(coord, target);
    CoordType source_center = get(coord, source);
    CoordType target_to_source = source_center - target_center;
    
    for( typename HoleList::iterator it = hole_list.begin(); it != hole_list.end(); ++it) {
      
      Vertex target_vertex = *it;
      Vertex source_vertex = get(inv_coord, get(coord, target_vertex) + target_to_source);
      
      // Let the visitor do the actual painting of the hole:
      vis.paint_vertex(target_vertex, source_vertex, g);
    };
    
    //finally, paint the center of the target patch:
    vis.paint_vertex(target, source, g);
    // Note, I do this last because painting the target-center might screw up the list of nearby-holes,
    //  since that list might be destroyed when the hole is painted.
    
  };
  
};

/**
 * This class template is a patch inpainter (i.e. paints the holes in one patch with the values of 
 * some given source patch). This class requires that the graph be a grid-graph (like BGL's grid_graph)
 * whose dimension is fixed at compile-time (dimensions = 2 means a 2D grid). This class uses the 
 * color-map that is given to it to check if the vertices in a patch are holes that need to be filled 
 * or not (i.e. it does a masking of the patch around a given target center vertex).
 * 
 * \tparam GridDimensions The dimensions of the grid (i.e. 2 means a 2D grid, 3 means a 3D grid, etc.).
 * \tparam ColorMap A property-map to obtain the vertex color for a given vertex descriptor.
 */
template <std::size_t GridDimensions, typename ColorMap>
struct masked_grid_patch_inpainter {
  std::size_t patch_half_width;
  ColorMap color;
  
  masked_grid_patch_inpainter(std::size_t aPatchWidth, ColorMap aColor) : patch_half_width(aPatchWidth / 2), color(aColor) { };
  
  template <std::size_t Dim, typename Vertex, typename GridGraph, typename InpaintingVisitor>
  typename boost::enable_if_c< (GridDimensions == Dim),
  void >::type loop(Vertex, Vertex, GridGraph&, InpaintingVisitor) { };
  
  template <std::size_t Dim, typename Vertex, typename GridGraph, typename InpaintingVisitor>
  typename boost::disable_if_c< (GridDimensions == Dim),
  void >::type loop(Vertex target, Vertex source, GridGraph& g, InpaintingVisitor vis) {
    Vertex target_left = target;
    Vertex source_left = source;
    for(std::size_t i = 0; i < patch_half_width; ++i) {
      target_left = g.next(target_left, Dim);
      source_left = g.next(source_left, Dim);
      // check the mask value:
      if( get(color, target_left) == Color::black() )
	vis.paint_vertex(target_left, source_left, g); //paint the vertex.
      // loop in the nested dimension:
      loop< Dim + 1 >(target_left, source_left, g, vis);
    };
    Vertex target_right = target;
    Vertex source_right = source;
    for(std::size_t i = 0; i < patch_half_width; ++i) {
      target_right = g.previous(target_right, Dim);
      source_right = g.previous(source_right, Dim);
      // check the mask value:
      if( get(color, target_right) == Color::black() )
	vis.paint_vertex(target_right, source_right, g); //paint the vertex.
      // loop in the nested dimension:
      loop< Dim + 1 >(target_right, source_right, g, vis);
    };
  };
  
  
  template <typename Vertex, typename GridGraph, typename InpaintingVisitor>
  void operator()(Vertex target, Vertex source, GridGraph& g, InpaintingVisitor vis) {
    
    using boost::get; 
    
    //first, paint the center of the target patch:
    vis.paint_vertex(target, source, g);
    
    // then, start the recursive looping (note, the recursion is at compile-time, so it is safe).
    loop<0>(target,source,g,vis);
    
  };
  
};


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
void inpainting_no_init(VertexListGraph& g, InpaintingVisitor vis,
		        const Topology& space, PositionMap position,
                        ColorMap color, PriorityMap priority, 
			PriorityCompare compare_priority,
		        NearestNeighborFinder find_inpainting_source, 
		        PatchInpainter inpaint_patch) {
  
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<InpaintingVisitor,VertexListGraph>))
  
  typedef typename boost::graph_traits<VertexListGraph>::vertex_descriptor Vertex;
  typedef typename boost::property_traits<ColorMap>::value_type ColorValue;
  typedef boost::color_traits<ColorValue> Color;
  typedef typename boost::property_traits<PriorityMap>::value_type PriorityValue;
  typedef boost::vector_property_map<std::size_t> IndexInHeapMap;
  IndexInHeapMap index_in_heap;
  {
    typename boost::graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = boost::vertices(g); ui != ui_end; ++ui) {
      put(index_in_heap,*ui, (std::size_t)(-1)); //this ugly C-style cast is required to match the boost::d_ary_heap_indirect implementation.
    };
  };
  
  typedef boost::d_ary_heap_indirect<Vertex, 4, IndexInHeapMap, PriorityMap, PriorityCompare> MutableQueue;
  MutableQueue Q(priority, index_in_heap, compare_priority); //priority queue
  
  // add all the black vertices (holes) to the priority-queue:
  {
    typename boost::graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = boost::vertices(g); ui != ui_end; ++ui) {
      if( get(color, *ui) == Color::black() )
        Q.push(*ui);  // the priority-queue will automatically get the priority and put the vertex in the right order.
    };
  };
  
  // start the in-painting loop:
  detail::inpainting_loop(g, detail::inpainting_visitor_wrapper<InpaintingVisitor,ColorMap>(vis,color),
			  space, position, color, Q, find_inpainting_source, inpaint_patch);
  
};


/**
 * This function template performs an in-painting of the hole-vertices of a graph.
 * This version performs an initialization of the vertices by calling upon the visitor 
 * to do so on all vertices. The visitor is responsible for correctly initializing the 
 * color value (black for holes, white otherwise), the priority value, and the 
 * position value, as well as any other values necessary for functors to function 
 * correctly.
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
void inpainting(VertexListGraph& g, InpaintingVisitor vis,
		const Topology& space, PositionMap position,
                ColorMap color, PriorityMap priority, 
		PriorityCompare compare_priority,
		NearestNeighborFinder find_inpainting_source, 
		PatchInpainter inpaint_patch) {
  
  typedef typename boost::graph_traits<VertexListGraph>::vertex_descriptor Vertex;
  
  // initialize all vertices:
  {
    typename boost::graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = boost::vertices(g); ui != ui_end; ++ui) {
      vis.initialize_vertex(*ui,g);
    };
  };
  
  // start the in-painting:
  inpainting_no_init(g, vis, space, position, color, priority, compare_priority, 
		     find_inpainting_source, inpaint_patch);
  
};


/**
 * This function template performs an in-painting of the hole-vertices of a graph,
 * and uses a DVP-tree to search for the source patches. For the rest, this function 
 * is the same as inpainting().
 * 
 * \tparam VertexListGraph A graph type that can hold a list of vertices.
 * \tparam InpaintingVisitor A visitor type that models the InpaintingVisitorConcept which is used to inject all the custom code into this algorithm.
 * \tparam Topology A topology type that can compute a distance-value between two positions.
 * \tparam PositionMap A property-map type that can fetch the positions associated to vertices.
 * \tparam ColorMap A property-map type that can fetch the color-values associated to vertices.
 * \tparam PriorityMap A property-map type that can fetch the priority-values associated to vertices.
 * \tparam PriorityCompare A functor type that can compare priority-values (strict weak ordering).
 * \tparam PatchInpainter A functor type that traverse the holes in a patch and apply a visitor painting on them.
 * \param g A graph that holds all the vertices.
 * \param vis A visitor to inject all the custom code into this algorithm.
 * \param space A topology to compute a distance-value between two positions.
 * \param position A property-map to fetch the positions associated to vertices.
 * \param color A property-map to fetch the color-values associated to vertices.
 * \param priority A property-map to fetch the priority-values associated to vertices.
 * \param compare_priority A functor to compare priority-values (strict weak ordering).
 * \param inpaint_patch A functor to traverse the holes in a patch and apply a visitor's painting on them.
 */
template <typename VertexListGraph, typename InpaintingVisitor,
          typename Topology, typename PositionMap,
          typename ColorMap, typename PriorityMap, 
	  typename PriorityCompare, 
	  typename PatchInpainter>
inline
void inpainting_with_dvp(VertexListGraph& g, InpaintingVisitor vis,
		         const Topology& space, PositionMap position,
                         ColorMap color, PriorityMap priority, 
		         PriorityCompare compare_priority, 
	                 PatchInpainter inpaint_patch) {
  
  typedef typename boost::graph_traits<VertexListGraph>::vertex_descriptor Vertex;
  
  // initialize all vertices:
  {
    typename boost::graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = boost::vertices(g); ui != ui_end; ++ui) {
      vis.initialize_vertex(*ui,g);
    };
  };
  
  // create a dvp-tree nearest-neighbor search tree:
  typedef ReaK::pp::dvp_tree<Vertex, Topology, PositionMap, 4> DVPTreeType;
  DVPTreeType nn_tree(g,space,position);
  ReaK::pp::multi_dvp_tree_search<VertexListGraph,DVPTreeType> nn_finder;
  nn_finder.graph_tree_map[&g] = &nn_tree;
  
  // start the in-painting:
  inpainting_no_init(g, vis, space, position, color, priority, compare_priority, 
		     nn_finder, inpaint_patch);
  
};


/**
 * This function template performs an in-painting of the hole-vertices of a graph,
 * uses a DVP-tree to search for the source patches, and uses a masked-grid traversal of
 * the holes in the grid when doing the actual painting of the holes. For the rest, this function 
 * is the same as inpainting().
 * 
 * \tparam Dimensions The dimensions of the graph (2 for 2D grid, etc.).
 * \tparam InpaintingVisitor A visitor type that models the InpaintingVisitorConcept which is used to inject all the custom code into this algorithm.
 * \tparam Topology A topology type that can compute a distance-value between two positions.
 * \tparam PositionMap A property-map type that can fetch the positions associated to vertices.
 * \tparam ColorMap A property-map type that can fetch the color-values associated to vertices.
 * \tparam PriorityMap A property-map type that can fetch the priority-values associated to vertices.
 * \tparam PriorityCompare A functor type that can compare priority-values (strict weak ordering).
 * \param g A graph that holds all the vertices.
 * \param vis A visitor to inject all the custom code into this algorithm.
 * \param space A topology to compute a distance-value between two positions.
 * \param position A property-map to fetch the positions associated to vertices.
 * \param color A property-map to fetch the color-values associated to vertices (black for holes, white otherwise).
 * \param priority A property-map to fetch the priority-values associated to vertices.
 * \param compare_priority A functor to compare priority-values (strict weak ordering).
 * \param patch_width The width of the patches when painted.
 */
template <std::size_t Dimensions, typename InpaintingVisitor,
          typename Topology, typename PositionMap,
          typename ColorMap, typename PriorityMap, 
	  typename PriorityCompare>
inline
void inpainting_grid_with_dvp(boost::grid_graph<Dimensions>& g, InpaintingVisitor vis,
		              const Topology& space, PositionMap position,
                              ColorMap color, PriorityMap priority, 
		              PriorityCompare compare_priority,
			      std::size_t patch_width) {
  
  inpainting_with_dvp(g, vis, space, position, color, priority, compare_priority, 
                      masked_grid_patch_inpainter< Dimensions, ColorMap>(patch_width, color));
  
};


#endif












