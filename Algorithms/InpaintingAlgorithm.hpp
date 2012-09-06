/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef InpaintingAlgorithm_hpp
#define InpaintingAlgorithm_hpp

// Concepts
#include "Concepts/InpaintingVisitorConcept.hpp"

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

// Custom
#include "BoostHelpers/BoostHelpers.h"

/** When this function is called, the priority-queue must already be filled with
  * all the boundary nodes (which should also have their boundaryStatusMap set appropriately).
  * The only thing this function does is run the inpainting loop. All of the
  * actual code is externalized in the functors and visitors (vis, find_inpainting_source,
  * inpaint_patch, etc.).
  */
// We declare this function here so we can use the gcc attribute keyword (it cannot be used on definitions)
//template <typename TVertexListGraph, typename TInpaintingVisitor,
//          typename TBoundaryStatusMap, typename TPriorityQueue,
//          typename TNearestNeighborFinder, typename TPatchInpainter>
//inline
//void InpaintingAlgorithm(TVertexListGraph& g, TInpaintingVisitor vis,
//                        TBoundaryStatusMap* boundaryStatusMap, TPriorityQueue* boundaryNodeQueue,
//                        TNearestNeighborFinder find_inpainting_source,
//                        TPatchInpainter* patchInpainter) __attribute__((optimize(0)));

template <typename TVertexListGraph, typename TInpaintingVisitor,
          typename TBoundaryStatusMap, typename TPriorityQueue,
          typename THandleMap, typename TPriorityMap,
          typename TNearestNeighborFinder, typename TPatchInpainter>
inline
void InpaintingAlgorithm(TVertexListGraph& g, TInpaintingVisitor vis,
                        TBoundaryStatusMap* boundaryStatusMap, TPriorityQueue* boundaryNodeQueue,
                        THandleMap& handleMap, TPriorityMap& priorityMap,
                        TNearestNeighborFinder find_inpainting_source,
                        TPatchInpainter* patchInpainter)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

//   std::cout << "At the beginning of the algorithm there are " << (*boundaryNodeQueue).size() << " nodes in the queue." << std::endl;
//   std::cout << "At the beginning of the algorithm there are "
//             << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//             << " valid nodes in the queue." << std::endl;

  unsigned int iteration = 0;
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
      if( boundaryNodeQueue->empty() )
      {
        std::cout << "Inpainting complete." << std::endl;
        vis.InpaintingComplete();
  //         std::cout << "(There are " << (*boundaryNodeQueue).size() << " nodes in the queue)." << std::endl;
        return;  //terminate if the queue is empty.
      }
      targetNode = boundaryNodeQueue->top();
      typename THandleMap::value_type invalidHandle(0);
      put(handleMap, targetNode, invalidHandle);
      boundaryNodeQueue->pop();
    } while( get(*boundaryStatusMap, targetNode) == false );

//    std::cout << "Processing node (" << targetNode[0] << ", " << targetNode[1] << ") with priority: " << get(priorityMap, targetNode) << std::endl;

//     std::cout << "Before DiscoverVertex there are " << boundaryNodeQueue->size()
//               << " nodes in the queue." << std::endl;
//     std::cout << "Before DiscoverVertex there are "
//               << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;

    // Notify the visitor that we have a hole target center.
    vis.DiscoverVertex(targetNode);

    // Find the source node that matches best to the target node
//     std::cout << "Before PotentialMatchMade there are " << boundaryNodeQueue->size()
//               << " nodes in the queue." << std::endl;
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
//     std::cout << "Before inpaint_patch there are " << (*boundaryNodeQueue).size()
//               << " nodes in the queue." << std::endl;
//     std::cout << "Before inpaint_patch there are "
//               << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(targetNode);
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);

    patchInpainter->PaintPatch(targetIndex, sourceIndex);
    // patchInpainter->PaintPatch(targetNode, sourceNode); // Ideally we would like to do this, but PatchInpainter::PaintPatch cannot be a template (c++ rules say so)

//     std::cout << "Before FinishVertex there are " << (*boundaryNodeQueue).size()
//               << " nodes in the queue." << std::endl;
//     std::cout << "Before FinishVertex there are "
//               << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;
    vis.FinishVertex(targetNode, sourceNode);

//     std::cout << "At the end of the iteration there are " << boundaryNodeQueue->size()
//               << " nodes in the queue." << std::endl;
//     std::cout << "At the end of the iteration there are "
//               << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//               << " valid nodes in the queue." << std::endl;

//    PatchHelpers::WriteValidQueueNodesPrioritiesImage(*boundaryNodeQueue, *boundaryStatusMap, priorityMap,
//                                              vis.GetImage()->GetLargestPossibleRegion(),
//                                              Helpers::GetSequentialFileName("QueueImagePriorities", iteration, "mha", 3));

//    PatchHelpers::WriteValidQueueNodesLocationsImage(*boundaryNodeQueue, *boundaryStatusMap,
//                                              vis.GetImage()->GetLargestPossibleRegion(),
//                                              Helpers::GetSequentialFileName("QueueImageLocations", iteration, "mha", 3));

    iteration++;
  } // end main iteration loop

}

#endif
