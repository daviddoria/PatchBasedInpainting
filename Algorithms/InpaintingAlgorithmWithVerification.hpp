#ifndef InpaintingAlgorithmWithVerification_hpp
#define InpaintingAlgorithmWithVerification_hpp

// Concepts
#include "Concepts/InpaintingVisitorConcept.hpp"

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

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
          typename TKNNFinder, typename TBestNeighborFinder, typename TManualNeighborFinder,
          typename TPatchInpainter>
inline
void InpaintingAlgorithmWithVerification(TVertexListGraph& g, TInpaintingVisitor vis,
                     TBoundaryStatusMap* boundaryStatusMap, TPriorityQueue* boundaryNodeQueue,
                     TKNNFinder knnFinder, TBestNeighborFinder& bestNeighborFinder,
                     TManualNeighborFinder& manualNeighborFinder, TPatchInpainter inpaint_patch)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

  unsigned int numberOfManualVerifications = 0;
  while(true)
  {
    // Find the next target to in-paint. Some of the nodes in the priority queue
    // can be already filled (they get "covered up" when a patch is filled around
    // a target node). So we do not just process the node at the front of the queue,
    // we also check that it has not been filled by looking at its value in the boundaryStatusMap

    VertexDescriptorType targetNode;
    do
    {
      if( (*boundaryNodeQueue).empty() )
      {
        std::cout << "Queue is empty, exiting." << std::endl;
        return;  //terminate if the queue is empty.
      }
      targetNode = (*boundaryNodeQueue).top();
      (*boundaryNodeQueue).pop();
    } while( get(*boundaryStatusMap, targetNode) == false);
    //} while( get(boundaryStatusMap, targetNode) == false && get(fillStatusMap, targetNode));

    // Notify the visitor that we have a hole target center.
    vis.DiscoverVertex(targetNode);

    // Find the source node that matches best to the target node
    typename boost::graph_traits<TVertexListGraph>::vertex_iterator vi,vi_end;
    tie(vi,vi_end) = vertices(g);

    std::vector<VertexDescriptorType> outputContainer;
    knnFinder(vi, vi_end, targetNode, outputContainer);

    VertexDescriptorType sourceNode = bestNeighborFinder(outputContainer.begin(), outputContainer.end(), targetNode);
    vis.PotentialMatchMade(targetNode, sourceNode);

    if(!vis.AcceptMatch(targetNode, sourceNode))
      {
      numberOfManualVerifications++;
      std::cout << "So far there have been " << numberOfManualVerifications << " manual verifications." << std::endl;
      std::cout << "Automatic match not accepted!" << std::endl;
      sourceNode = manualNeighborFinder(outputContainer.begin(), outputContainer.end(), targetNode);
      }

    // Do the in-painting of the target patch from the source patch.
    // the inpaint_patch functor should take care of calling
    // "vis.paint_vertex(target, source)" on the individual vertices in the patch.
    inpaint_patch(targetNode, sourceNode, vis);

    vis.FinishVertex(targetNode, sourceNode);
  } // end main iteration loop

  std::cout << "There were " << numberOfManualVerifications << " manual verifications required." << std::endl;
  vis.InpaintingComplete();

};

#endif
