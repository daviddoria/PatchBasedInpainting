#ifndef InpaintingAlgorithm_hpp
#define InpaintingAlgorithm_hpp

// Concepts
#include "Concepts/InpaintingVisitorConcept.hpp"

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

// TODO: Finish this if it turns out to be necessary.
/** This function is different from InpaintingAlgorithm() in that it handles the case where
  * a best patch should not be used.
  * When this function is called, the priority-queue must already be filled with
  * all the boundary nodes (which should also have their boundaryStatusMap set appropriately).
  * The only thing this function does is run the inpainting loop. All of the
  * actual code is externalized in the functors and visitors (vis, find_inpainting_source,
  * inpaint_patch, etc.).
  */
template <typename TVertexListGraph, typename TInpaintingVisitor,
          typename TBoundaryStatusMap, typename TPriorityQueue,
          typename TKNNFinder, typename TBestNeighborFinder, typename TPatchInpainter>
inline
void InpaintingAlgorithmWithVerification(TVertexListGraph& g, TInpaintingVisitor vis,
                     TBoundaryStatusMap& boundaryStatusMap, TPriorityQueue& boundaryNodeQueue,
                     TKNNFinder topPatchesFinder, TBestNeighborFinder& manualSelectionVisitor,
                     TPatchInpainter inpaint_patch)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

  while(true)
  {
    // Find the next target to in-paint. Some of the nodes in the priority queue
    // can be already filled (they get "covered up" when a patch is filled around
    // a target node). So we do not just process the node at the front of the queue,
    // we also check that it has not been filled (by looking at its boundaryStatusMap
    // value).
    VertexDescriptorType targetNode;
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
    typename boost::graph_traits<TVertexListGraph>::vertex_iterator vi,vi_end;
    tie(vi,vi_end) = vertices(g);

    std::vector<VertexDescriptorType> outputContainer;
    this->MultipleNeighborFinder(first, last, queryNode, outputContainer);
    
    VertexDescriptorType sourceNode = find_inpainting_source(vi, vi_end, targetNode);
    vis.vertex_match_made(targetNode, sourceNode, g);

    if(!vis.accept_match(targetNode, g))
      {
      std::cout << "Match not accepted!" << std::endl;
      sourceNode = manualSelectionVisitor.select(targetNode, vi, vi_end);
      }

    // Do the in-painting of the target patch from the source patch.
    // the inpaint_patch functor should take care of calling
    // "vis.paint_vertex(target, source, g)" on the individual vertices in the patch.
    inpaint_patch(targetNode, sourceNode, g, vis);

    vis.finish_vertex(targetNode, sourceNode, g);
  } // end main iteration loop

  vis.inpainting_complete();

};

#endif
