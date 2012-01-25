#ifndef MaskedGridPatchInpainter_HPP
#define MaskedGridPatchInpainter_HPP

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

#endif
