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

#ifndef ImagePatchPixelDescriptor_hxx
#define ImagePatchPixelDescriptor_hxx

#include "ImagePatchPixelDescriptor.h" // Appease syntax parser

#include "ImageProcessing/Mask.h"

#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"

template <typename TImage>
ImagePatchPixelDescriptor<TImage>::ImagePatchPixelDescriptor() : Image(NULL), FullyValid(false), InsideImage(false)
{

}

template <typename TImage>
ImagePatchPixelDescriptor<TImage>::ImagePatchPixelDescriptor(TImage* const image, Mask* const maskImage, const itk::ImageRegion<2>& region) :
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
    this->SetStatus(SOURCE_PATCH);
    }
  else
    {
    this->SetStatus(INVALID);
    }
}
/*
template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::VisitAllPixels(const TImage* const image, PixelVisitor<typename TImage::PixelType> &visitor)
{
  itk::ImageRegionConstIterator<TImage> imageIterator(image, this->Region);

  while(!imageIterator.IsAtEnd())
    {
    visitor.Visit(imageIterator.Get());
    ++imageIterator;
    }
}*/

template <typename TImage>
bool ImagePatchPixelDescriptor<TImage>::IsFullyValid() const
{
  return this->FullyValid;
}

template <typename TImage>
bool ImagePatchPixelDescriptor<TImage>::IsInsideImage() const
{
  return this->InsideImage;
}

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetImage(const TImage* const image)
{
  this->Image = image;
}

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetRegion(const itk::ImageRegion<2>& region)
{
  this->Region = region;
}

template <typename TImage>
TImage* ImagePatchPixelDescriptor<TImage>::GetImage() const
{
  return this->Image;
}
/*
template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::VisitAllValidPixels(const TImage* const image, const Mask* const mask, PixelVisitor<typename TImage::PixelType> &visitor)
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
void ImagePatchPixelDescriptor<TImage>::VisitOffsets(const TImage* const image, const std::vector<itk::Offset<2> >& offsets, PixelVisitor<typename TImage::PixelType> &visitor)
{
  itk::Index<2> corner = this->Region.GetIndex();

  for(unsigned int offsetId = 0; offsetId < offsets.size(); ++offsetId)
    {
    visitor.Visit(image->GetPixel(corner + offsets[offsetId]));
    }
}*/

template <typename TImage>
itk::Index<2> ImagePatchPixelDescriptor<TImage>::GetCorner() const
{
  return this->Region.GetIndex();
}

template <typename TImage>
itk::ImageRegion<2> ImagePatchPixelDescriptor<TImage>::GetRegion() const
{
  return this->Region;
}

template <typename TImage>
std::ostream& operator<<(std::ostream& output, const ImagePatchPixelDescriptor<TImage> &patch)
{
  output << "Patch: " << patch.GetRegion() << std::endl;
  return output;
}

template <typename TImage>
bool ImagePatchPixelDescriptor<TImage>::operator==(const ImagePatchPixelDescriptor& other) const
{
  if(this->Region == other.Region)
    {
    return true;
    }
  return false;
}

template <typename TImage>
bool ImagePatchPixelDescriptor<TImage>::operator!=(const ImagePatchPixelDescriptor& other) const
{
  return !operator==(other);
}
/*
template <typename TImage>
bool ImagePatchPixelDescriptor<TImage>::operator<(const ImagePatchPixelDescriptor& other) const
{
  // TODO: Use the itk::Index LexicalCompare functor
  if(this->Region.GetIndex()[0] < other.Region.GetIndex()[0])
    {
    return true;
    }
  else if (other.Region.GetIndex()[0] < this->Region.GetIndex()[0])
    {
    return false;
    }

  if (this->Region.GetIndex()[1] < other.Region.GetIndex()[1])
    {
    return true;
    }
  else if (other.Region.GetIndex()[1] < this->Region.GetIndex()[1])
    {
    return false;
    }
  assert(0); // This should never be reached
  return true;
}*/

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetValidOffsets(const std::vector<itk::Offset<2> >& validOffsets)
{
  this->ValidOffsets = validOffsets;
}

#endif
