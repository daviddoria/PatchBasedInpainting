#ifndef SimpleInitializer_HPP
#define SimpleInitializer_HPP

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Priority/Priority.h"

// Debug
#include "Helpers/HelpersOutput.h"

template <typename TVisitor, typename TGraph>
inline void SimpleInitializer(Mask* const maskImage, TVisitor* const visitor, TGraph& g)
{
  std::cout << "SimpleInitializer" << std::endl;

  itk::ImageRegionConstIteratorWithIndex<Mask> maskIterator(maskImage, maskImage->GetLargestPossibleRegion());
  while(!maskIterator.IsAtEnd())
    {
    typename boost::graph_traits<TGraph>::vertex_descriptor node;
    node[0] = maskIterator.GetIndex()[0];
    node[1] = maskIterator.GetIndex()[1];
    //std::cout << "Initializing " << maskIterator.GetIndex() << std::endl;
    visitor->initialize_vertex(node, g);

    ++maskIterator;
    }

}

#endif
