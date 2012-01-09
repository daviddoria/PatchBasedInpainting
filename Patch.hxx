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

#include "Patch.h" // Make syntax parser happy

#include "Mask.h"

#include "itkImageRegionConstIteratorWithIndex.h"

template <typename TImage>
void Patch::VisitAllPixels(const TImage* const image, PixelVisitor<typename TImage::PixelType> &visitor)
{
  itk::ImageRegionConstIterator<TImage> imageIterator(image, this->Region);

  while(!imageIterator.IsAtEnd())
    {
    visitor.Visit(imageIterator.Get());
    ++imageIterator;
    }
}

template <typename TImage>
void Patch::VisitAllValidPixels(const TImage* const image, const Mask* const mask, PixelVisitor<typename TImage::PixelType> &visitor)
{
  itk::ImageRegionConstIteratorWithIndex<TImage> imageIterator(image, this->Region);

  while(!imageIterator.IsAtEnd())
    {
    if(mask->IsValid(imageIterator.GetIndex()))
      {
      visitor.Visit(imageIterator.Get());
      }
    ++imageIterator;
    }
}

template <typename TImage>
void Patch::VisitOffsets(const TImage* const image, const std::vector<itk::Offset<2> >& offsets, PixelVisitor<typename TImage::PixelType> &visitor)
{
  itk::Index<2> corner = this->Region.GetIndex();

  for(unsigned int offsetId = 0; offsetId < offsets.size(); ++offsetId)
    {
    visitor.Visit(image->GetPixel(corner + offsets[offsetId]));
    }
}
