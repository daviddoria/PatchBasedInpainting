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
          typename TPriorityQueue, typename TKNNFinder, typename TBestNeighborFinder,
          typename TManualNeighborFinder, typename TPatchInpainter>
inline
void InpaintingAlgorithmWithVerification(TVertexListGraph& g, TInpaintingVisitor vis,
                     TPriorityQueue* boundaryNodeQueue,
                     TKNNFinder knnFinder, TBestNeighborFinder& bestNeighborFinder,
                     TManualNeighborFinder* manualNeighborFinder, TPatchInpainter* patchInpainter)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  std::cout << "Enter InpaintingAlgorithmWithVerification..." << std::endl;

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

  unsigned int numberOfManualVerifications = 0;

  // Find the next target to in-paint. Some of the nodes in the priority queue
  // can be already filled (they get "covered up" when a patch is filled around
  // a target node). So we do not just process the node at the front of the queue,
  // we also check that it has not been filled by looking at its value in the boundaryStatusMap

  while(!boundaryNodeQueue->empty())
  {
    std::cout << "Starting iteration..." << std::endl;
    VertexDescriptorType targetNode = boundaryNodeQueue->top(); // This also pops the node

    // Notify the visitor that we have a hole target center.
    vis.DiscoverVertex(targetNode);

    // Find the source node that matches best to the target node
    typename boost::graph_traits<TVertexListGraph>::vertex_iterator vi,vi_end;
    tie(vi,vi_end) = vertices(g);

    std::vector<VertexDescriptorType> outputContainer(knnFinder.GetK());
    knnFinder(vi, vi_end, targetNode, outputContainer.begin());

    VertexDescriptorType sourceNode = bestNeighborFinder(outputContainer.begin(), outputContainer.end(), targetNode);
    vis.PotentialMatchMade(targetNode, sourceNode);

    if(!vis.AcceptMatch(targetNode, sourceNode))
    {
      numberOfManualVerifications++;
      std::cout << "So far there have been " << numberOfManualVerifications << " manual verifications." << std::endl;
      std::cout << "Automatic match not accepted!" << std::endl;
      sourceNode = (*manualNeighborFinder)(outputContainer.begin(), outputContainer.end(), targetNode);
    }

    // Do the in-painting of the target patch from the source patch.
    // the inpaint_patch functor should take care of calling
    // "vis.paint_vertex(target, source)" on the individual vertices in the patch.
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(targetNode);
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);

    patchInpainter->PaintPatch(targetIndex, sourceIndex);

    vis.FinishVertex(targetNode, sourceNode);
  }

  std::cout << "Inpainting complete. There were " << numberOfManualVerifications << " manual verifications required." << std::endl;
  vis.InpaintingComplete();
}

#endif
