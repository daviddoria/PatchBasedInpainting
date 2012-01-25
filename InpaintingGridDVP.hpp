#ifndef InpaintingGridDVP_HPP
#define InpaintingGridDVP_HPP

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


