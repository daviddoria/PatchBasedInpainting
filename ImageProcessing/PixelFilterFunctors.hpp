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

#ifndef PixelFilterFunctors_H
#define PixelFilterFunctors_H

// Custom
#include "Mask.h"

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"

template<typename TImage, typename TFunctor>
std::vector<itk::Index<2> > PixelsSatisfyingFunctor(const TImage* const image,
                                                    const itk::ImageRegion<2>& region, TFunctor& functor)
{
  std::vector<itk::Index<2> > passingPixels;

  itk::ImageRegionConstIteratorWithIndex<TImage> regionIterator(image, region);

  while(!regionIterator.IsAtEnd())
    {
    if(functor(regionIterator.Get()))
      {
      passingPixels.push_back(regionIterator.GetIndex());
      }
    ++regionIterator;
    }

  return passingPixels;
}

template<typename TPixel>
class GreaterThanFunctor
{
public:

  GreaterThanFunctor(const float threshold) : Threshold(threshold)
  {
  }

  /** Determine if a pixel is greater than the threshold */
  bool operator()(const TPixel& pixel)
  {
    if(pixel > Threshold)
      {
      return true;
      }
    return false;
  }

private:
  TPixel Threshold;

};

#endif
