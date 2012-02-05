#ifndef InitializeFromMaskImage_HPP
#define InitializeFromMaskImage_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Priority/Priority.h"

// Debug
#include "Helpers/HelpersOutput.h"

template <typename TVisitor, typename TGraph, typename TFillStatusMap>
inline void InitializeFromMaskImage(Mask* const maskImage, TVisitor* const visitor, TGraph& g,
                                    TFillStatusMap& fillStatusMap)
{
  std::cout << "InitializeFromMaskImage" << std::endl;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;
  // Initialize the fill status
  itk::ImageRegionConstIteratorWithIndex<Mask> maskIterator(maskImage, maskImage->GetLargestPossibleRegion());
  while(!maskIterator.IsAtEnd())
    {
    VertexDescriptorType node;
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

  // Intialize all nodes
  itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(maskImage,
                                                             maskImage->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    VertexDescriptorType node;
    node[0] = imageIterator.GetIndex()[0];
    node[1] = imageIterator.GetIndex()[1];

    visitor->initialize_vertex(node, g);

    ++imageIterator;
    }

}

#endif
