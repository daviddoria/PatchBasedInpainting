#ifndef InitializeFromMaskImage_HPP
#define InitializeFromMaskImage_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Priority/Priority.h"

// Debug
#include "Helpers/OutputHelpers.h"

template <typename TVisitor, typename TNode>
inline void InitializeFromMaskImage(Mask* const maskImage, TVisitor* const visitor)
{
  std::cout << "InitializeFromMaskImage" << std::endl;

  // Intialize all nodes
  itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(maskImage,
                                                             maskImage->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    TNode node = Helpers::ConvertFrom<TNode, itk::Index<2> >(imageIterator.GetIndex());

    visitor->InitializeVertex(node);

    ++imageIterator;
    }

}

#endif
