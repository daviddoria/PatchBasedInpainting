#ifndef InitializeFromMaskImage_HPP
#define InitializeFromMaskImage_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Priority/Priority.h"

// Debug
#include "Helpers/HelpersOutput.h"

template <typename TBoundaryNodeQueue, typename TPriorityMap, typename TVisitor, typename TGraph, typename TFillStatusMap>
inline void InitializeFromMaskImage(Mask* const maskImage, TBoundaryNodeQueue* const boundaryNodeQueue, TPriorityMap* const priorityMap,
                                    Priority* const priorityFunction, TVisitor* const visitor, TGraph* const g,
                                    TFillStatusMap* const fillStatusMap)
{
  // Initialize the fill status
  itk::ImageRegionConstIteratorWithIndex<Mask> maskIterator(maskImage, maskImage->GetLargestPossibleRegion());
  while(!maskIterator.IsAtEnd())
    {
    typename TBoundaryNodeQueue::value_type node;
    node[0] = maskIterator.GetIndex()[0];
    node[1] = maskIterator.GetIndex()[1];
    if(maskImage->IsHole(maskIterator.GetIndex()))
      {
      put(*fillStatusMap, node, false);
      }
    else
      {
      put(*fillStatusMap, node, true);
      }
    ++maskIterator;
    }

  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  maskImage->FindBoundary(boundaryImage);

  HelpersOutput::WriteImage(maskImage, "mask.png");
  HelpersOutput::WriteImage(boundaryImage.GetPointer(), "boundary.png");

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

  std::cout << "There are " << boundaryNodeQueue->size() << " nodes in the boundaryNodeQueue" << std::endl;

}

#endif
