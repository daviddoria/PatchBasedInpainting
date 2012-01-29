#ifndef InitializeFromMaskImage_HPP
#define InitializeFromMaskImage_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Priority/Priority.h"

// Debug
#include "Helpers/HelpersOutput.h"

template <typename TBoundaryNodeQueue, typename TPriorityMap, typename TVisitor, typename TGraph, typename TFillStatusMap, typename TBoundaryStatusMap>
inline void InitializeFromMaskImage(Mask* const maskImage, TBoundaryNodeQueue& boundaryNodeQueue, TPriorityMap& priorityMap,
                                    Priority* const priorityFunction, TVisitor* const visitor, TGraph& g,
                                    TFillStatusMap& fillStatusMap, TBoundaryStatusMap& boundaryStatusMap)
{
  std::cout << "InitializeFromMaskImage" << std::endl;
  // Initialize the fill status
  itk::ImageRegionConstIteratorWithIndex<Mask> maskIterator(maskImage, maskImage->GetLargestPossibleRegion());
  while(!maskIterator.IsAtEnd())
    {
    typename TBoundaryNodeQueue::value_type node;
    node[0] = maskIterator.GetIndex()[0];
    node[1] = maskIterator.GetIndex()[1];
    if(maskImage->IsHole(maskIterator.GetIndex()))
      {
      put(fillStatusMap, node, false);
      }
    else
      {
      put(fillStatusMap, node, true);
      }
    ++maskIterator;
    }

  // Compute the boundary image
  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  maskImage->FindBoundary(boundaryImage);

  HelpersOutput::WriteImage(maskImage, "mask.png");
  HelpersOutput::WriteImage(boundaryImage.GetPointer(), "boundary.png");

  // Add boundary nodes to the queue, compute their priority, and set their boundary status to true.
  itk::ImageRegionConstIteratorWithIndex<Mask::BoundaryImageType> imageIterator(boundaryImage, boundaryImage->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    typename TBoundaryNodeQueue::value_type node;
    node[0] = imageIterator.GetIndex()[0];
    node[1] = imageIterator.GetIndex()[1];

    visitor->initialize_vertex(node, g);

    if(imageIterator.Get() != 0) // boundary pixel found
      {
      boundaryNodeQueue.push(node);

      put(boundaryStatusMap, node, true);

      itk::Index<2> index;
      index[0] = node[0];
      index[1] = node[1];
      float priority = priorityFunction->ComputePriority(index);
      std::cout << "initial priority: " << priority << std::endl;
      put(priorityMap, node, priority);
      }
    else
      {
      put(boundaryStatusMap, node, false);
      }
    ++imageIterator;
    }

  std::cout << "InitializeFromMaskImage: There are " << boundaryNodeQueue.size() << " nodes in the boundaryNodeQueue" << std::endl;

}

#endif
