#ifndef InitializePriority_HPP
#define InitializePriority_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Priority/Priority.h"

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
    typename TBoundaryNodeQueue::value_type node;
    node[0] = imageIterator.GetIndex()[0];
    node[1] = imageIterator.GetIndex()[1];

    if(imageIterator.Get() != 0) // boundary pixel found
      {
      boundaryNodeQueue.push(node);

      put(boundaryStatusMap, node, true);

      itk::Index<2> index;
      index[0] = node[0];
      index[1] = node[1];
      float priority = priorityFunction->ComputePriority(index);
      // std::cout << "initial priority: " << priority << std::endl;
      put(priorityMap, node, priority);
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
