#ifndef InpaintingAlgorithmWithLocalSearch_hpp
#define InpaintingAlgorithmWithLocalSearch_hpp

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
          typename TBoundaryStatusMap, typename TPriorityQueue, typename TSearchRegion,
          typename TKNNFinder, typename TBestNeighborFinder, typename TManualNeighborFinder,
          typename TPatchInpainter>
inline
void InpaintingAlgorithmWithLocalSearch(TVertexListGraph& g, TInpaintingVisitor vis,
                        TBoundaryStatusMap* boundaryStatusMap, TPriorityQueue* boundaryNodeQueue,
                        TSearchRegion& searchRegion,
                        TKNNFinder knnFinder, TBestNeighborFinder& bestNeighborFinder,
                        TManualNeighborFinder& manualNeighborFinder,
                        TPatchInpainter inpaint_patch)
{
  BOOST_CONCEPT_ASSERT((InpaintingVisitorConcept<TInpaintingVisitor, TVertexListGraph>));

  typedef typename boost::graph_traits<TVertexListGraph>::vertex_descriptor VertexDescriptorType;

  while(true)
  {
    VertexDescriptorType targetNode;
    do
    {
      if( (*boundaryNodeQueue).empty() )
      {
        std::cout << "Inpainting complete." << std::endl;
        vis.InpaintingComplete();

        return;  //terminate if the queue is empty.
      }
      targetNode = (*boundaryNodeQueue).top();
      (*boundaryNodeQueue).pop();
    } while( get(*boundaryStatusMap, targetNode) == false );

    // Notify the visitor that we have a hole target center.
    vis.DiscoverVertex(targetNode);

    std::vector<VertexDescriptorType> searchRegionNodes = searchRegion(targetNode);

    std::vector<VertexDescriptorType> outputContainer(knnFinder.GetK());
    knnFinder(searchRegionNodes.begin(), searchRegionNodes.end(), targetNode, outputContainer.begin());
    outputContainer.resize(searchRegionNodes.size());

    VertexDescriptorType sourceNode = bestNeighborFinder(outputContainer.begin(), outputContainer.end(), targetNode);

    vis.PotentialMatchMade(targetNode, sourceNode);

    if(!vis.AcceptMatch(targetNode, sourceNode))
      {
      std::cout << "Automatic match not accepted!" << std::endl;

      // If the match was not accepted automatically, search the full image.
      typename boost::graph_traits<TVertexListGraph>::vertex_iterator vi,vi_end;
      tie(vi,vi_end) = vertices(g);
      std::vector<VertexDescriptorType> fullSearchOutputContainer(knnFinder.GetK());
      knnFinder(vi, vi_end, targetNode, fullSearchOutputContainer.begin());
      sourceNode = bestNeighborFinder(fullSearchOutputContainer.begin(), fullSearchOutputContainer.end(), targetNode);

      // If the match is still not acceptable, allow the user to choose a patch manually
      if(!vis.AcceptMatch(targetNode, sourceNode))
        {
        sourceNode = manualNeighborFinder(fullSearchOutputContainer.begin(), fullSearchOutputContainer.end(), targetNode);
        }
      }
      
    inpaint_patch(targetNode, sourceNode, vis);

    vis.FinishVertex(targetNode, sourceNode);

  } // end main iteration loop

};

#endif
