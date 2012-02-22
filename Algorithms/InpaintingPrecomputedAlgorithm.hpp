#ifndef InpaintingAlgorithm_hpp
#define InpaintingAlgorithm_hpp

// Boost
#include <boost/graph/properties.hpp>

// STL
#include <stdexcept>

// Custom
#include "Helpers/BoostHelpers.h"

template <typename TNodeQueue, typename TNearestNeighborFinder, typename TPatchInpainter>
inline
void InpaintingAlgorithm(TNodeQueue& nodeQueue,
                        TNearestNeighborFinder find_inpainting_source,
                        TPatchInpainter inpaint_patch)
{
  while(!nodeQueue->empty())
  {
    VertexDescriptorType targetNode = nodeQueue->top();
    VertexDescriptorType sourceNode = find_inpainting_source(nodeQueue.begin(), nodeQueue.end(), targetNode);

    inpaint_patch(targetNode, sourceNode, vis);

  } // end main iteration loop

  vis.InpaintingComplete();

};

#endif
