/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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

#include "ImageProcessing/PixelFilterFunctors.hpp"

// ITK
#include "itkImageRegionIteratorWithIndex.h"

// STL
#include <iostream>
#include <sstream>
#include <stdexcept>

int main()
{
  typedef itk::Image<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();

  itk::Index<2> corner = {{0,0}};
  itk::Size<2> size = {{10,10}};
  itk::ImageRegion<2> region(corner, size);
  image->SetRegions(region);
  image->Allocate();

  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < 5)
      {
      imageIterator.Set(100);
      }
    else
      {
      imageIterator.Set(1);
      }
    ++imageIterator;
    }

  GreaterThanFunctor<ImageType::PixelType> greaterThanFunctor(5);
  std::vector<itk::Index<2> > pixelsSatisfyingFunctor = PixelsSatisfyingFunctor(image.GetPointer(),
                                                      image->GetLargestPossibleRegion(), greaterThanFunctor);

  const unsigned int expectedNumberOfPixels = 50;
  if(pixelsSatisfyingFunctor.size() != expectedNumberOfPixels)
    {
    std::stringstream ss;
    ss << "There were " << pixelsSatisfyingFunctor.size() << " pixels that satisfied the functor, but there should have been "
       << expectedNumberOfPixels;
    throw std::runtime_error(ss.str());
    }
  return EXIT_SUCCESS;
}
