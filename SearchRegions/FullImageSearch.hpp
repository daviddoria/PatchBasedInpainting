#ifndef FullImageSearch_HPP
#define FullImageSearch_HPP

// Custom
#include "Helpers.h"

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImage.h"
/**

 */
template <typename TVertexDescriptorType>
struct FullImageSearch
{
  itk::ImageRegion<2> FullRegion;

  typedef std::vector<TVertexDescriptorType> VectorType;
  
  VectorType Vertices;
    
  FullImageSearch(const itk::ImageRegion<2>& fullRegion) : FullRegion(fullRegion)
  {
    // We have to create a dummy image with the specified region in order to use the region iterator
    typedef itk::Image<int, 2> ImageType;
    ImageType::Pointer image = ImageType::New();
    image->SetRegions(FullRegion);
    image->Allocate();

    Vertices.resize(FullRegion.GetNumberOfPixels());
    unsigned int counter = 0;
    itk::ImageRegionConstIteratorWithIndex<ImageType> imageIterator(image, FullRegion);
    while(!imageIterator.IsAtEnd())
      {
      TVertexDescriptorType vert = Helpers::ConvertFrom<TVertexDescriptorType, itk::Index<2> > (imageIterator.GetIndex());
      Vertices[counter] = vert;
      counter++;
      ++imageIterator;
      }
  };

  VectorType operator()(const TVertexDescriptorType& center)
  {
    // Do nothing, we already composed the vector of all vertices in the constructor
    return Vertices;
  }

};

#endif
