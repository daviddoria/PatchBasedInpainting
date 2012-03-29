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

#ifndef ImagePatchVectorized_hpp
#define ImagePatchVectorized_hpp

#include "ImagePatchVectorized.h" // Appease syntax parser

#include "Mask/Mask.h"

#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"

template <typename TImage>
ImagePatchVectorized<TImage>::ImagePatchVectorized() : Image(NULL), FullyValid(false), InsideImage(false)
{

}

template <typename TImage>
ImagePatchVectorized<TImage>::ImagePatchVectorized(TImage* const image, Mask* const maskImage, const itk::ImageRegion<2>& region) :
Region(region), Image(image), MaskImage(maskImage), InsideImage(false)
{
  if(image->GetLargestPossibleRegion().IsInside(region))
    {
    this->InsideImage = true;
    }
  else
    {
    this->FullyValid = false;
    return;
    }

  this->FullyValid = maskImage->IsValid(region);
  this->FullyValid = true;
  itk::ImageRegionConstIteratorWithIndex<Mask> maskIterator(maskImage, region);
  while(!maskIterator.IsAtEnd())
    {
    if(maskImage->IsHole(maskIterator.GetIndex()))
      {
      this->FullyValid = false;
      break;
      }

    ++maskIterator;
    }

  if(this->FullyValid)
    {
    this->SetStatus(SOURCE_NODE);
    CreatePixelVector();
    }
  else
    {
    this->SetStatus(INVALID);
    }
}

template <typename TImage>
void ImagePatchVectorized<TImage>::CreatePixelVector()
{
  itk::ImageRegionConstIterator<ImageType> iterator(Image, Region);

  PixelVector.resize(Region.GetNumberOfPixels());
  unsigned int linearCounter = 0;
  while(!iterator.IsAtEnd())
    {
    PixelVector[linearCounter] = iterator.Get();
    linearCounter++;
    ++iterator;
    }
}

template <typename TImage>
bool ImagePatchVectorized<TImage>::IsFullyValid() const
{
  return this->FullyValid;
}

template <typename TImage>
bool ImagePatchVectorized<TImage>::IsInsideImage() const
{
  return this->InsideImage;
}

template <typename TImage>
void ImagePatchVectorized<TImage>::SetImage(const TImage* const image)
{
  this->Image = image;
}

template <typename TImage>
void ImagePatchVectorized<TImage>::SetRegion(const itk::ImageRegion<2>& region)
{
  this->Region = region;
}

template <typename TImage>
TImage* ImagePatchVectorized<TImage>::GetImage() const
{
  return this->Image;
}

template <typename TImage>
itk::Index<2> ImagePatchVectorized<TImage>::GetCorner() const
{
  return this->Region.GetIndex();
}

template <typename TImage>
itk::ImageRegion<2> ImagePatchVectorized<TImage>::GetRegion() const
{
  return this->Region;
}

template <typename TImage>
std::ostream& operator<<(std::ostream& output, const ImagePatchVectorized<TImage> &patch)
{
  output << "Patch: " << patch.GetRegion() << std::endl;
  return output;
}

template <typename TImage>
void ImagePatchVectorized<TImage>::SetValidOffsets(const std::vector<unsigned int>& validOffsets)
{
  this->ValidOffsets = validOffsets;
}

#endif
