#ifndef InpaintingAlgorithm_hpp
#define InpaintingAlgorithm_hpp

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
                      PatchInpainter inpaint_patch) 
{
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

#endif
