/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef FullImageSearch_HPP
#define FullImageSearch_HPP

// Custom
#include <Helpers/Helpers.h>

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
  }

  VectorType operator()(const TVertexDescriptorType& center)
  {
    // Do nothing but return the result, we already composed the vector of all vertices in the constructor
    return this->Vertices;
  }

};

#endif
