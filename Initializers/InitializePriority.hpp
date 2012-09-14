#ifndef InitializePriority_HPP
#define InitializePriority_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "Mask/Mask.h"
#include "Priority/Priority.h"
#include "ITKHelpers/ITKHelpers.h"

template <typename TBoundaryNodeQueue, typename TPriority>
inline void InitializePriority(Mask* const maskImage, TBoundaryNodeQueue& boundaryNodeQueue,
                               TPriority* const priorityFunction)
{
  std::cout << "InitializePriority" << std::endl;

  typedef typename TBoundaryNodeQueue::HandleType HandleType;

  // Compute the boundary image
  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  unsigned char boundaryPixelValue = 255;
  maskImage->CreateBoundaryImage(boundaryImage, Mask::VALID, boundaryPixelValue);

  //   ITKHelpers::WriteImage(maskImage, "mask.png");
  //   ITKHelpers::WriteImage(boundaryImage.GetPointer(), "boundary.png");

  // Add boundary nodes to the queue, compute their priority, and set their boundary status to true.
  itk::ImageRegionConstIteratorWithIndex<Mask::BoundaryImageType> boundaryImageIterator(boundaryImage,
                                                                                boundaryImage->GetLargestPossibleRegion());
  while(!boundaryImageIterator.IsAtEnd())
  {
    typename TBoundaryNodeQueue::ValueType node =
        Helpers::ConvertFrom<typename TBoundaryNodeQueue::ValueType,
                              itk::Index<2> >(boundaryImageIterator.GetIndex());

    if(boundaryImageIterator.Get() == boundaryPixelValue)
    {
      itk::Index<2> index = ITKHelpers::CreateIndex(node);

      float priority = priorityFunction->ComputePriority(index);
//      std::cout << "initial priority for node (" << node[0] << ", " << node[1] << "): " << priority << std::endl;
      put(boundaryNodeQueue.PriorityMap, node, priority);

      // Note: the priorityMap value must be set before pushing the node into the queue
      // (as the indirect queue is using the value from the map to determine the node's position)
      HandleType handle = boundaryNodeQueue.push(node);
      put(boundaryNodeQueue.HandleMap, node, handle);

      put(boundaryNodeQueue.BoundaryStatusMap, node, true);
    }
    else
    {
      put(boundaryNodeQueue.BoundaryStatusMap, node, false);
    }
    ++boundaryImageIterator;
  }

  std::cout << "InitializePriority: There are " << boundaryNodeQueue.CountValidNodes()
            << " nodes in the boundaryNodeQueue" << std::endl;

}

#endif
