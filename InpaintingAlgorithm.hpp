#ifndef InpaintingAlgorithm_hpp
#define InpaintingAlgorithm_hpp

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

template <typename VertexListGraph, typename InpaintingVisitorType,
          typename BoundaryStatusMap, typename PriorityQueue, 
          typename NearestNeighborFinder, typename PatchInpainter>
inline
void inpainting_loop(VertexListGraph& g, InpaintingVisitorType vis,
                      BoundaryStatusMap& boundaryStatusMap, PriorityQueue& boundaryNodeQueue,
                      NearestNeighborFinder find_inpainting_source, 
                      PatchInpainter inpaint_patch) 
{
  typedef typename boost::graph_traits<VertexListGraph>::vertex_descriptor Vertex;

  // When this function is called, the priority-queue should already be filled 
  // with all the hole-vertices (which should also have their boundaryStatusMap set appropriately).
  // The only thing this function does is run the inpainting loop. All of the
  // actual code is externalized in the functors and visitors (vis, find_inpainting_source,
  // inpaint_patch, etc.).

  while(true) 
  {
    // Find the next target to in-paint. Some of the nodes in the priority queue
    // can be already filled (they get "covered up" when a patch is filled around
    // a target node). So we do not just process the node at the front of the queue,
    // we also check that it has not been filled (by looking at its boundaryStatusMap
    // value).
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

    // Find the source node that matches best to the target node
    Vertex source_patch_center = find_inpainting_source(targetNode);
    vis.vertex_match_made(targetNode, source_patch_center, g);

    // Do the in-painting of the target patch from the source patch.
    // the inpaint_patch functor should take care of iterating through the vertices in both
    // patches and call "vis.paint_vertex(target, source, g)" on the individual vertices.
    inpaint_patch(targetNode, source_patch_center, g, vis);

    if(!vis.accept_painted_vertex(targetNode, g))
      {
      throw std::runtime_error("Vertex was not painted successfully!");
      }

    vis.finish_vertex(targetNode, g);
  } // end main iteration loop

};

#endif
