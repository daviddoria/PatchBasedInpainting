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
//   this->FullyValid = true;
//   itk::ImageRegionConstIteratorWithIndex<Mask> maskIterator(maskImage, region);
//   while(!maskIterator.IsAtEnd())
//     {
//     if(maskImage->IsHole(maskIterator.GetIndex()))
//       {
//       this->FullyValid = false;
//       break;
//       }
// 
//     ++maskIterator;
//     }
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

////////////// Non-member functions ////////////////
template <typename TImage>
float Compare(const ImagePatchPixelDescriptor<TImage>* const a, const ImagePatchPixelDescriptor<TImage>* const b)
{
  // This comparison must allow source patches to be compared to source patches (to create the tree) as well as source patches
  // to be symmetrically compared to target patches.

  assert(a->IsInsideImage());
  assert(b->IsInsideImage());
  assert(a->GetImage() == b->GetImage());

  // If either patch is not entirely inside the image, the comparison cannot be performed.
//   if(!this->IsInsideImage() || !other->IsInsideImage())
//     {
//     return std::numeric_limits<float>::infinity();
//     }

  // We allow 'this' to be invalid but not 'other' because we want to
  // compare target patches that definitely have invalid (hole) pixels to completely valid patches.
//   if(!other->IsValid())
//     {
//     //std::cout << "Invalid difference comparison!" << std::endl;
//     return std::numeric_limits<float>::infinity();
//     }

  TImage* image = a->GetImage(); // For now this image is required to be the same for both patches.

  float totalDifference = 0.0f;

  itk::Offset<2> offsetAToB = b->GetCorner() - a->GetCorner();

  // Compare all corresponding pixels and sum their differences
  itk::ImageRegionConstIteratorWithIndex<TImage> imageIterator(image, a->GetRegion());
  while(!imageIterator.IsAtEnd())
    {
    //float difference = fabs(imageIterator.Get() - this->Image->GetPixel(imageIterator.GetIndex() + offsetToOther));
    float difference = (imageIterator.Get() - image->GetPixel(imageIterator.GetIndex() + offsetAToB)).GetNorm();
    totalDifference += difference;

    ++imageIterator;
    }
  //std::cout << "Difference: " << totalDifference << std::endl;
  return totalDifference;
}

// template <typename TImage>
// float ImagePatchPixelDescriptor<TImage>::Compare(const ImagePatchPixelDescriptor* const other, const std::vector<itk::Offset<2> >& offsets) const
// {
//   if(!this->Valid || !other->IsValid())
//     {
//     return std::numeric_limits<float>::infinity();
//     }
// 
//   float totalDifference = 0.0f;
// 
//   for(unsigned int offsetId = 0; offsetId < offsets.size(); ++offsetId)
//   {
//     float difference = fabs(this->Image->GetPixel(this->GetCorner() + offsets[offsetId]) - this->Image->GetPixel(other->GetCorner() + offsets[offsetId]));
//     totalDifference += difference;
//   }
// 
//   return totalDifference;
// }

#endif
