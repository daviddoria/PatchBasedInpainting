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

#include "ValidPixelIterator.h" // Appease syntax parser

#include "itkImageRegionConstIteratorWithIndex.h"

template <typename TImage>
ValidPixelIterator<TImage>::ValidPixelIterator(const TImage* const image, const itk::ImageRegion<2>& region):
Image(image)
{
  itk::ImageRegionConstIteratorWithIndex<TImage> iterator(image, region);

  // Create all of the valid regions and store them in a vector
  while(!iterator.IsAtEnd())
    {
    if(iterator.Get() != NULL)
      {
      this->ValidPixels.push_back(iterator.GetIndex());
      }

    ++iterator;
    }
}

template <typename TImage>
typename ValidPixelIterator<TImage>::ConstIterator ValidPixelIterator<TImage>::begin() const
{
  return this->ValidPixels.begin();
}

template <typename TImage>
typename ValidPixelIterator<TImage>::ConstIterator ValidPixelIterator<TImage>::end() const
{
  return this->ValidPixels.end();
}
