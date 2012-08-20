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

#include "Visitors/InpaintingVisitorConcept.hpp"

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

template <typename VertexListGraphType, typename InpaintingVisitorType,
          typename BoundaryStatusMapType, typename PriorityQueueType, 
          typename NearestNeighborFinderType, typename PatchInpainterType>
inline
void inpainting_loop(VertexListGraphType& g, InpaintingVisitorType vis,
                     BoundaryStatusMapType& boundaryStatusMap, PriorityQueueType& boundaryNodeQueue,
                     NearestNeighborFinderType find_inpainting_source, 
                     PatchInpainterType inpaint_patch) 
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<InpaintingVisitorType, VertexListGraphType>));
  
  typedef typename boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

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
    
    // TODO: Find the top N nodes to potentially inpaint.
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

    // TODO: Find the source node that matches best to the target node for all of the potential nodes to inpaint.
    typename boost::graph_traits<VertexListGraphType>::vertex_iterator vi,vi_end;
    tie(vi,vi_end) = vertices(g);
    VertexDescriptorType sourceNode = find_inpainting_source(vi, vi_end, targetNode);
    vis.vertex_match_made(targetNode, sourceNode, g);

    inpaint_patch(targetNode, sourceNode, g, vis);

    if(!vis.accept_painted_vertex(targetNode, g))
      {
      throw std::runtime_error("Vertex was not painted successfully!");
      }

    vis.finish_vertex(targetNode, sourceNode, g);
  } // end main iteration loop

};

#endif
