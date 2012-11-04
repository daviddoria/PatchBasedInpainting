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
#include <memory>
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
void InpaintingAlgorithmWithVerification(std::shared_ptr<TVertexListGraph> graph,
                                         std::shared_ptr<TInpaintingVisitor> visitor,
                                         std::shared_ptr<TPriorityQueue> boundaryNodeQueue,
                                         std::shared_ptr<TKNNFinder> knnFinder,
                                         std::shared_ptr<TBestNeighborFinder> bestNeighborFinder,
                                         std::shared_ptr<TManualNeighborFinder> manualNeighborFinder,
                                         std::shared_ptr<TPatchInpainter> patchInpainter)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  std::cout << "Enter InpaintingAlgorithmWithVerification..." << std::endl;

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

  unsigned int numberOfManualVerifications = 0;

  while(!boundaryNodeQueue->empty())
  {
    std::cout << "Starting iteration..." << std::endl;
    VertexDescriptorType targetNode = boundaryNodeQueue->top(); // This also pops the node

    // Notify the visitor that we have a hole target center.
    visitor->DiscoverVertex(targetNode);

    // Find the source node that matches best to the target node
    typedef typename boost::graph_traits<TVertexListGraph>::vertex_iterator VertexIterator;
    VertexIterator graphBegin;
    VertexIterator graphEnd;
    tie(graphBegin, graphEnd) = vertices(*graph);

    // Allocate the container of K nearest neighbors
    std::vector<VertexDescriptorType> knnContainer(knnFinder->GetK());

    // Find the K nearest neighbors
    (*knnFinder)(graphBegin, graphEnd, targetNode, knnContainer.begin());

    // Find the best neighbor out of the top K (probably using a different criterion)
    VertexDescriptorType sourceNode = (*bestNeighborFinder)(knnContainer.begin(),
                                                            knnContainer.end(), targetNode);

    // Broadcast that we have found a potential match
    visitor->PotentialMatchMade(targetNode, sourceNode);

    // If the acceptance tests do not pass, ask the user for a better patch
    if(!visitor->AcceptMatch(targetNode, sourceNode))
    {
      numberOfManualVerifications++;
      std::cout << "So far there have been "
                << numberOfManualVerifications << " manual verifications." << std::endl;
      std::cout << "Automatic match not accepted!" << std::endl;
      sourceNode = (*manualNeighborFinder)(knnContainer.begin(), knnContainer.end(), targetNode);
    }

    // Do the in-painting of the target patch from the source patch.
    // the inpaint_patch functor should take care of calling
    // "vis.paint_vertex(target, source)" on the individual vertices in the patch.
    itk::Index<2> targetIndex = ITKHelpers::CreateIndex(targetNode);
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);

    // Inpaint the target patch
    patchInpainter->PaintPatch(targetIndex, sourceIndex);

    // Broadcast that we are done with this iteration
    visitor->FinishVertex(targetNode, sourceNode);
  }

  std::cout << "Inpainting complete. There were "
            << numberOfManualVerifications << " manual verifications required." << std::endl;
  visitor->InpaintingComplete();
}

#endif
