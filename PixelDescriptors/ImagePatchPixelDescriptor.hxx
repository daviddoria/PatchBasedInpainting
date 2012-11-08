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

// Submodules
#include <Mask/Mask.h>

// ITK
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"

template <typename TImage>
ImagePatchPixelDescriptor<TImage>::ImagePatchPixelDescriptor()
{

}

template <typename TImage>
ImagePatchPixelDescriptor<TImage>::ImagePatchPixelDescriptor(TImage* const image,
                                                             Mask* const maskImage, const itk::ImageRegion<2>& region) :
  PixelDescriptor(), Region(region), OriginalRegion(region), Image(image), MaskImage(maskImage)
{
  if(image->GetLargestPossibleRegion().IsInside(region))
  {
    this->InsideImage = true;

    this->FullyValid = maskImage->IsValid(region);

    if(this->FullyValid)
    {
      this->SetStatus(SOURCE_NODE);
    }
  }
}

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetFullyValid(const bool fullyValid)
{
  this->FullyValid = fullyValid;
}

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetInsideImage(const bool insideImage)
{
  this->InsideImage = insideImage;
}

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
void ImagePatchPixelDescriptor<TImage>::SetOriginalRegion(const itk::ImageRegion<2>& originalRegion)
{
  this->OriginalRegion = originalRegion;
}

template <typename TImage>
TImage* ImagePatchPixelDescriptor<TImage>::GetImage() const
{
  return this->Image;
}

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
itk::ImageRegion<2> ImagePatchPixelDescriptor<TImage>::GetOriginalRegion() const
{
  return this->OriginalRegion;
}

template <typename TImage>
std::ostream& operator<<(std::ostream& output, const ImagePatchPixelDescriptor<TImage> &patch)
{
  output << "Patch: " << patch.GetRegion() << std::endl;
  return output;
}

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetValidOffsets(const std::vector<itk::Offset<2> >& validOffsets)
{
  this->ValidOffsets = validOffsets;
}

#endif
