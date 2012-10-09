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

#ifndef InpaintingAlgorithmWithLocalSearch_hpp
#define InpaintingAlgorithmWithLocalSearch_hpp

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
          typename TPriorityQueue, typename TSearchRegion,
          typename TPatchInpainter, typename TBestPatchFinder>
inline void
InpaintingAlgorithmWithLocalSearch(TVertexListGraph& g, TInpaintingVisitor vis,
                                   TPriorityQueue* boundaryNodeQueue,
                                   TBestPatchFinder bestPatchFinder,
                                   TPatchInpainter* patchInpainter,
                                   TSearchRegion& searchRegion, const bool debug = false)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

  unsigned int iteration = 0;
  while(!boundaryNodeQueue->empty())
  {
    std::cout << "Algorithm: Iteration " << iteration << std::endl;

    VertexDescriptorType targetNode = boundaryNodeQueue->top();

    if(debug)
    {
      std::cout << "targetNode: " << targetNode[0] << ", " << targetNode[1] << std::endl;
    }

    // Notify the visitor that we have a hole target center.
    vis.DiscoverVertex(targetNode);

    // Create a list of the source patches to search
    std::vector<VertexDescriptorType> searchRegionNodes = searchRegion(targetNode);

    VertexDescriptorType sourceNode = bestPatchFinder(searchRegionNodes.begin(),
                                                      searchRegionNodes.end(), targetNode);

    vis.PotentialMatchMade(targetNode, sourceNode);

    // Inpaint the target patch from the source patch.
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(targetNode);
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);

    patchInpainter->PaintPatch(targetIndex, sourceIndex);

    vis.FinishVertex(targetNode, sourceNode);

    iteration++;
  }

  std::cout << "Inpainting complete." << std::endl;
  vis.InpaintingComplete();
}

#endif
