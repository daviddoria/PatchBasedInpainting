#ifndef InitializePriority_HPP
#define InitializePriority_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Priority/Priority.h"
#include "Helpers/ITKHelpers.h"

// Debug
#include "Helpers/OutputHelpers.h"

template <typename TBoundaryNodeQueue, typename TPriorityMap, typename TBoundaryStatusMap, typename TPriority>
inline void InitializePriority(Mask* const maskImage, TBoundaryNodeQueue& boundaryNodeQueue, TPriorityMap& priorityMap,
                               TPriority* const priorityFunction, 
                               TBoundaryStatusMap& boundaryStatusMap)
{
  std::cout << "InitializePriority" << std::endl;

  // Compute the boundary image
  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  maskImage->FindBoundary(boundaryImage);

//   HelpersOutput::WriteImage(maskImage, "mask.png");
//   HelpersOutput::WriteImage(boundaryImage.GetPointer(), "boundary.png");

  // Add boundary nodes to the queue, compute their priority, and set their boundary status to true.
  itk::ImageRegionConstIteratorWithIndex<Mask::BoundaryImageType> imageIterator(boundaryImage,
                                                                                boundaryImage->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    typename TBoundaryNodeQueue::value_type node = Helpers::ConvertFrom<typename TBoundaryNodeQueue::value_type,
           itk::Index<2> >(imageIterator.GetIndex());

    if(imageIterator.Get() != 0) // boundary pixel found
      {
      itk::Index<2> index = ITKHelpers::CreateIndex(node);

      float priority = priorityFunction->ComputePriority(index);
      // std::cout << "initial priority: " << priority << std::endl;
      put(priorityMap, node, priority);

      // Note: the priorityMap value must be set before pushing the node into the queue
      // (as the indirect queue is using the value from the map to determine the node's position)
      boundaryNodeQueue.push(node);

      put(boundaryStatusMap, node, true);
      }
    else
      {
      put(boundaryStatusMap, node, false);
      }
    ++imageIterator;
    }

  std::cout << "InitializePriority: There are " << boundaryNodeQueue.size()
            << " nodes in the boundaryNodeQueue" << std::endl;

}

#endif
