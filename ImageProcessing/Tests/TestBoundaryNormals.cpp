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

#include "BoundaryNormals.h"

// ITK
#include "itkImage.h"

// Submodules
#include <Mask/Mask.h>

// STL
#include <iostream>
#include <stdexcept>

static void TestBoundaryNormals();

template <typename TImage>
static void CreateImage(TImage* const image);

static void CreateMask(Mask* const mask);

int main()
{
  TestBoundaryNormals();

  return EXIT_SUCCESS;
}

// Look from a pixel across the hole in a specified direction and return the pixel that exists on the other side of the hole.
void TestBoundaryNormals()
{
  typedef itk::Image<unsigned char, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  CreateImage(image.GetPointer());
  ITKHelpers::WriteImage(image.GetPointer(), "Image.mha");

  Mask::Pointer mask = Mask::New();
  CreateMask(mask);
  ITKHelpers::WriteImage(mask.GetPointer(), "Mask.mha");

  BoundaryNormals boundaryNormals(mask);

  typedef itk::Image<itk::CovariantVector<float, 2>, 2> BoundaryNormalsImageType;
  BoundaryNormalsImageType::Pointer boundaryNormalsImage = BoundaryNormalsImageType::New();
  float blurVariance = 2.0f;
  boundaryNormals.ComputeBoundaryNormals(blurVariance, boundaryNormalsImage.GetPointer());

  ITKHelpers::WriteImage(boundaryNormalsImage.GetPointer(), "BoundaryNormals_Blur2.mha");

  // Try the function with no blurring
  boundaryNormals.ComputeBoundaryNormals(0.0f, boundaryNormalsImage.GetPointer()); // 0.0f for "no blur variance"
  ITKHelpers::WriteImage(boundaryNormalsImage.GetPointer(), "BoundaryNormals_Blur0.mha");
}

template <typename TImage>
void CreateImage(TImage* const image)
{
  typename TImage::IndexType corner;
  corner.Fill(0);

  typename TImage::SizeType size;
  size.Fill(100);

  typename TImage::RegionType region(corner, size);

  image->SetRegions(region);
  image->Allocate();

  itk::ImageRegionIterator<TImage> imageIterator(image,region);

  while(!imageIterator.IsAtEnd())
  {
//    if(imageIterator.GetIndex()[0] < 70)
//    {
//      imageIterator.Set(255);
//    }
//    else
//    {
//      imageIterator.Set(0);
//    }

    imageIterator.Set(rand() % 255);
    ++imageIterator;
  }

}

void CreateMask(Mask* const mask)
{
  typename Mask::IndexType corner;
  corner.Fill(0);

  typename Mask::SizeType size;
  size.Fill(100);

  typename Mask::RegionType region(corner, size);

  mask->SetRegions(region);
  mask->Allocate();

  itk::ImageRegionIterator<Mask> maskIterator(mask, region);

  while(!maskIterator.IsAtEnd())
  {
    if(maskIterator.GetIndex()[0] < 70)
    {
      maskIterator.Set(mask->GetValidValue());
    }
    else
    {
      maskIterator.Set(mask->GetHoleValue());
    }

    ++maskIterator;
  }

}
