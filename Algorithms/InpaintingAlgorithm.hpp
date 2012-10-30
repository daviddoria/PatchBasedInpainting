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
#include <BoostHelpers/BoostHelpers.h>

/** When this function is called, the priority-queue must already be filled with
  * all the boundary nodes (which should also have their boundaryStatusMap set appropriately).
  * The only thing this function does is run the inpainting loop. All of the
  * actual code is externalized in the functors and visitors (vis, find_inpainting_source,
  * inpaint_patch, etc.).
  */
template <typename TVertexListGraph, typename TInpaintingVisitor,
          typename TPriorityQueue, typename TBestPatchFinder,
          typename TPatchInpainter>
inline void
InpaintingAlgorithm(TVertexListGraph& g, TInpaintingVisitor vis,
                    TPriorityQueue* boundaryNodeQueue,
                    TBestPatchFinder bestPatchFinder,
                    TPatchInpainter* patchInpainter)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

//   std::cout << "At the beginning of the algorithm there are " << (*boundaryNodeQueue).size() << " nodes in the queue." << std::endl;
//   std::cout << "At the beginning of the algorithm there are "
//             << BoostHelpers::CountValidQueueNodes(*boundaryNodeQueue, *boundaryStatusMap)
//             << " valid nodes in the queue." << std::endl;

  unsigned int iteration = 0;
  while(!boundaryNodeQueue->empty())
  {
    VertexDescriptorType targetNode = boundaryNodeQueue->top(); // This also pops the node

    // Notify the visitor that we have a hole target center.
    vis.DiscoverVertex(targetNode);

    // Create a list of the source patches to search (all of them)
    typename boost::graph_traits<TVertexListGraph>::vertex_iterator vi,vi_end;
    tie(vi,vi_end) = vertices(g);

    // Find the source node that matches best to the target node
    VertexDescriptorType sourceNode = bestPatchFinder(vi, vi_end, targetNode);
    vis.PotentialMatchMade(targetNode, sourceNode);

    // Inpaint the target patch from the source patch.
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(targetNode);
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);

    patchInpainter->PaintPatch(targetIndex, sourceIndex);
    // Ideally we would like to do this, but PatchInpainter::PaintPatch
    // cannot be a template because it is virtual (c++ rules say so)
    // patchInpainter->PaintPatch(targetNode, sourceNode);

    vis.FinishVertex(targetNode, sourceNode);

    iteration++;
  } // end main iteration loop

  std::cout << "Inpainting complete." << std::endl;
  vis.InpaintingComplete();
}

#endif
