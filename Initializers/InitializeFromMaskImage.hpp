#ifndef InitializeFromMaskImage_HPP
#define InitializeFromMaskImage_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Priority/Priority.h"

template <typename TBoundaryNodeQueue, typename TPriorityMap, typename TVisitor, typename TGraph>
inline void InitializeFromMaskImage(Mask* const maskImage, TBoundaryNodeQueue* const boundaryNodeQueue, TPriorityMap* const priorityMap, Priority* const priorityFunction, TVisitor* const visitor, TGraph* const g)
{
  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  maskImage->FindBoundary(boundaryImage);

  itk::ImageRegionConstIteratorWithIndex<Mask::BoundaryImageType> imageIterator(boundaryImage, boundaryImage->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {

    if(imageIterator.Get() != 0) // boundary pixel found
      {
      typename TBoundaryNodeQueue::value_type node;
      node[0] = imageIterator.GetIndex()[0];
      node[1] = imageIterator.GetIndex()[1];

      visitor->initialize_vertex(node, *g);

      boundaryNodeQueue->push(node);

      itk::Index<2> index;
      index[0] = node[0];
      index[1] = node[1];
      put(*priorityMap, node, priorityFunction->ComputePriority(index));
      }
    ++imageIterator;
    }
}

#endif
