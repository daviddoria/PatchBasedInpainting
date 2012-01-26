#ifndef InitializeBoundaryQueueFromMaskImage_HPP
#define InitializeBoundaryQueueFromMaskImage_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"

template <typename TBoundaryNodeQueue>
inline void InitializeBoundaryQueueFromMaskImage(Mask* const maskImage, TBoundaryNodeQueue* const boundaryNodeQueue)
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
      boundaryNodeQueue->push(node);
      }
    ++imageIterator;
    }
}

#endif
