#ifndef InpaintingAlgorithm_hpp
#define InpaintingAlgorithm_hpp

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

template <typename VertexListGraph, typename InpaintingVisitorType,
          typename Topology, typename PositionMap,
          typename BoundaryStatusMap, typename PriorityQueue, 
          typename NearestNeighborFinder, 
          typename PatchInpainter>
inline
void inpainting_loop(VertexListGraph& g, InpaintingVisitorType vis,
                      const Topology& space, PositionMap& positionMap,
                      BoundaryStatusMap& boundaryStatusMap, PriorityQueue& boundaryNodeQueue,
                      NearestNeighborFinder find_inpainting_source, 
                      PatchInpainter inpaint_patch) 
{
  typedef typename boost::graph_traits<VertexListGraph>::vertex_descriptor Vertex;
  typedef typename boost::graph_traits<VertexListGraph>::edge_descriptor Edge;
  typedef typename boost::property_traits<PositionMap>::value_type PositionValueType;
  
  // When this function is called, the priority-queue should already be filled 
  // with all the hole-vertices (which should also have "Color::black()" color value).
  // So, the only thing this function does is run the inpainting loop. All of the
  // actual code is externalized in the functors and visitors (vis, find_inpainting_source, inpaint_patches, etc.).

  while(true) 
  {
    // find the next target to in-paint:
    Vertex targetNode;
    do
    {
      if( boundaryNodeQueue.empty() )
      {
        std::cout << "Queue is empty, exiting." << std::endl;
        return;  //terminate if the queue is empty.
      }
      targetNode = boundaryNodeQueue.top();
      boundaryNodeQueue.pop();
    } while( get(boundaryStatusMap, targetNode) == false );

    // Notify the visitor that we have a hole target center.
    vis.discover_vertex(targetNode, g);

    // next, we want to find a source patch-center that matches best to our target-patch:
    //  the nearest-neighbor finder will possibly need the target-patch, the graph, the topology
    //  and a property-map to get the "position" value ("position" is a point in the topology).
    //  Note, this is the standard form of my nearest-neighbor finders (any, linear, approximate, DVP-tree, etc.).
    PositionValueType targetPosition = get(positionMap, targetNode);
    Vertex source_patch_center = find_inpainting_source( targetPosition, g, space, positionMap );
    vis.vertex_match_made(targetNode, source_patch_center, g);

    // finally, do the in-painting of the target patch from the source patch.
    //  the inpaint_patch functor should take care of iterating through the vertices in both
    //  patches and call "vis.paint_vertex(target, source, g)" on the individual vertices.
    inpaint_patch(targetNode, source_patch_center, g, vis);

    if(!vis.accept_painted_vertex(targetNode, g))
      {
      throw std::runtime_error("Vertex was not painted successfully!");
      }

    vis.finish_vertex(targetNode, g);
  } // end main iteration loop
  
};

#endif
