#ifndef InpaintingAlgorithm_hpp
#define InpaintingAlgorithm_hpp

// Concepts
#include "Concepts/InpaintingVisitorConcept.hpp"

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

// Custom
#include "Helpers/BoostHelpers.h"

/** When this function is called, the priority-queue must already be filled with
  * all the boundary nodes (which should also have their boundaryStatusMap set appropriately).
  * The only thing this function does is run the inpainting loop. All of the
  * actual code is externalized in the functors and visitors (vis, find_inpainting_source,
  * inpaint_patch, etc.).
  */
template <typename TVertexListGraph, typename TInpaintingVisitor,
          typename TBoundaryStatusMap, typename TPriorityQueue,
          typename TNearestNeighborFinder, typename TPatchInpainter>
inline
void InpaintingAlgorithm(TVertexListGraph& g, TInpaintingVisitor vis,
                        TBoundaryStatusMap* boundaryStatusMap, TPriorityQueue* boundaryNodeQueue,
                        TNearestNeighborFinder find_inpainting_source,
                        TPatchInpainter inpaint_patch)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

//   std::cout << "At the beginning of the algorithm there are " << (*boundaryNodeQueue).size() << " nodes in the queue." << std::endl;
//   std::cout << "At the beginning of the algorithm there are "
//             << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//             << " valid nodes in the queue." << std::endl;

  while(true) 
  {
    // Find the next target to in-paint. Some of the nodes in the queue
    // can be already filled (they get "covered up" when a patch is filled around
    // a target node). So we do not just process the node at the front of the queue,
    // we also check that it has not been filled (by looking at its boundaryStatusMap
    // value).
    VertexDescriptorType targetNode;
    do
    {
      //std::cout << "There are " << boundaryNodeQueue.size() << " nodes in the queue." << std::endl;
      if( (*boundaryNodeQueue).empty() )
      {
      std::cout << "Inpainting complete." << std::endl;
      vis.InpaintingComplete();
//         std::cout << "(There are " << (*boundaryNodeQueue).size() << " nodes in the queue)." << std::endl;
        return;  //terminate if the queue is empty.
      }
      targetNode = (*boundaryNodeQueue).top();
      (*boundaryNodeQueue).pop();
    } while( get(*boundaryStatusMap, targetNode) == false );

//     std::cout << "Before DiscoverVertex there are " << (*boundaryNodeQueue).size() << " nodes in the queue." << std::endl;
//     std::cout << "Before DiscoverVertex there are "
//               << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;
    // Notify the visitor that we have a hole target center.
    vis.DiscoverVertex(targetNode);

    // Find the source node that matches best to the target node
//     std::cout << "Before PotentialMatchMade there are " << (*boundaryNodeQueue).size() << " nodes in the queue." << std::endl;
//     std::cout << "Before PotentialMatchMade there are "
//               << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;
    typename boost::graph_traits<TVertexListGraph>::vertex_iterator vi,vi_end;
    tie(vi,vi_end) = vertices(g);
    VertexDescriptorType sourceNode = find_inpainting_source(vi, vi_end, targetNode);
    vis.PotentialMatchMade(targetNode, sourceNode);

    // Do the in-painting of the target patch from the source patch.
    // the inpaint_patch functor should take care of calling
    // "vis.paint_vertex(target, source, g)" on the individual vertices in the patch.
//     std::cout << "Before inpaint_patch there are " << (*boundaryNodeQueue).size() << " nodes in the queue." << std::endl;
//     std::cout << "Before inpaint_patch there are " << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;
    inpaint_patch(targetNode, sourceNode, vis);

//     std::cout << "Before FinishVertex there are " << (*boundaryNodeQueue).size() << " nodes in the queue." << std::endl;
//     std::cout << "Before FinishVertex there are " << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;
    vis.FinishVertex(targetNode, sourceNode);

//     std::cout << "At the end of the iteration there are " << (*boundaryNodeQueue).size() << " nodes in the queue." << std::endl;
//     std::cout << "At the end of the iteration there are "
//               << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;
  } // end main iteration loop

};

#endif
