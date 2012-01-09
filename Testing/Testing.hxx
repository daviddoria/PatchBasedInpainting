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

#include "Testing.h" // Make syntax parser happy

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace Testing
{

template<typename T>
void OutputArray(const T* const a, const unsigned int length)
{
  for(unsigned int component = 0; component < length; ++component)
    {
    std::cout << static_cast<float>(a[component]) << " ";
    }
}

template<typename T>
std::string ArrayString(const T* const a, const unsigned int length)
{
  std::stringstream ss;
  ss << "(";
  for(unsigned int component = 0; component < length; ++component)
    {
    ss << static_cast<float>(a[component]);
    if(component != length - 1)
      {
      ss << ", ";
      }
    else
      {
      ss << ")";
      }
    }
  return ss.str();
}

template<typename T>
bool ArraysEqual(const T* const a, const T* const b, const unsigned int length, const float epsilon = 1e-6)
{
  for(unsigned int component = 0; component < 3; ++component)
    {
    if(!ValuesEqual(a[component], b[component]))
      {
      return false;
      }
    }
  return true;
}

template<typename T>
bool VectorsEqual(const std::vector<T>& a, const std::vector<T>& b)
{
  if(a.size() != b.size())
    {
    return false;
    }
  for(unsigned int i = 0; i < a.size(); ++i)
    {
    if(!ValuesEqual(a[i], b[i]))
      {
      return false;
      }
    }
  return true;
}

template<typename TImage>
bool ImagesEqual(const TImage* const image1, const TImage* const image2)
{
  if(image1->GetLargestPossibleRegion() != image2->GetLargestPossibleRegion())
    {
    return false;
    }
  itk::ImageRegionConstIterator<TImage> iterator1(image1, image1->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<TImage> iterator2(image2, image2->GetLargestPossibleRegion());
  while(!iterator1.IsAtEnd())
    {
    if(iterator1.Get() != iterator2.Get())
      {
      //std::cerr << "image 1 pixel: " << iterator1.Get() << " image 2 pixel: " << iterator2.Get() << std::endl;
      return false;
      }
    ++iterator1;
    ++iterator2;
    }
  return true;
}

template<typename TImage>
void GetBlankImage(TImage* const image)
{
  itk::Index<2> corner;
  corner.Fill(0);

  itk::Size<2> size;
  size.Fill(100);

  itk::ImageRegion<2> region(corner, size);
  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(itk::NumericTraits<typename TImage::PixelType>::Zero);
}

template<typename TImage>
void GetBlankImage(TImage* image, const unsigned int numberOfComponents)
{
  itk::Index<2> corner;
  corner.Fill(0);

  itk::Size<2> size;
  size.Fill(100);

  itk::ImageRegion<2> region(corner, size);
  image->SetRegions(region);
  image->SetNumberOfComponentsPerPixel(numberOfComponents);
  image->Allocate();
  itk::VariableLengthVector<typename TImage::InternalPixelType> v(numberOfComponents);
  v.Fill(0);
  //image->FillBuffer(itk::NumericTraits<typename TImage::PixelType>::Zero);
  image->FillBuffer(v);
}

template<typename TImage>
void GetHalfConstantImage(TImage* const image, const typename TImage::PixelType& leftSideConstant, const typename TImage::PixelType& rightSideConstant)
{
  GetBlankImage(image);
  itk::ImageRegionIterator<TImage> iterator(image, image->GetLargestPossibleRegion());
  while(!iterator.IsAtEnd())
    {
    if(static_cast<unsigned int>(iterator.GetIndex()[0]) < TestImageSize/2)
      {
      iterator.Set(leftSideConstant);
      }
    else
      {
      iterator.Set(rightSideConstant);
      }

    ++iterator;
    }
}

}// end namespace
