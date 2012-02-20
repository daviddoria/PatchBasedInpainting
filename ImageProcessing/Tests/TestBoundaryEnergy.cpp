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

#include "BoundaryEnergy.h"

// Custom
#include "Mask.h"
#include "Helpers/ITKHelpers.h"
#include "Testing/Testing.h"

// ITK
#include "itkVectorImage.h"

// STL
#include <iostream>
#include <stdexcept>

static void TestBoundaryEnergyScalarImage();
static void TestBoundaryEnergyVectorImage();

int main()
{
  TestBoundaryEnergyScalarImage();
  TestBoundaryEnergyVectorImage();

  return EXIT_SUCCESS;
}

void TestBoundaryEnergyScalarImage()
{
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{10,10}};
  itk::ImageRegion<2> imageRegion(imageCorner, imageSize);

  typedef itk::Image<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(imageRegion);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(imageRegion);
  mask->Allocate();

  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < 5)
      {
      imageIterator.Set(100);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetHoleValue());
      }
    else
      {
      imageIterator.Set(75);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetValidValue());
      }
    ++imageIterator;
    }

  BoundaryEnergy<ImageType> boundaryEnergy(image, mask);

//   itk::Index<2> regionCorner = {{5,5}};
//   itk::Size<2> regionSize = {{1, 1}};
//   itk::ImageRegion<2> region(regionCorner, regionSize);

  itk::Index<2> queryPixel = {{5,5}};
  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, 1);
  
  float energy = boundaryEnergy(region);
  std::cout << "Energy: " << energy << std::endl;

  float expectedEnergy = 25;
  if(energy != expectedEnergy)
  {
    std::stringstream ss;
    ss << "TestBoundaryEnergyScalarImage: Energy was " << energy << " but should have been " << expectedEnergy;
    throw std::runtime_error(ss.str());
  }
}


void TestBoundaryEnergyVectorImage()
{
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{10,10}};
  itk::ImageRegion<2> imageRegion(imageCorner, imageSize);

  const unsigned int imagePixelDimension = 3;
  typedef itk::VectorImage<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetNumberOfComponentsPerPixel(imagePixelDimension);
  image->SetRegions(imageRegion);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(imageRegion);
  mask->Allocate();

  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  ImageType::PixelType pixelA;
  pixelA.SetSize(imagePixelDimension);
  pixelA.Fill(100);

  ImageType::PixelType pixelB;
  pixelB.SetSize(imagePixelDimension);
  pixelB.Fill(75);

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < 5)
      {
      imageIterator.Set(pixelA);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetHoleValue());
      }
    else
      {
      imageIterator.Set(pixelB);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetValidValue());
      }
    ++imageIterator;
    }

  BoundaryEnergy<ImageType> boundaryEnergy(image, mask);

  itk::Index<2> queryPixel = {{5,5}};
  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, 1);

  float energy = boundaryEnergy(region);
  std::cout << "Energy: " << energy << std::endl;

  float expectedEnergy = (pixelA - pixelB).GetNorm();
  if(!Testing::ValuesEqual(energy, expectedEnergy))
  {
    std::stringstream ss;
    ss << "TestBoundaryEnergyVectorImage: Energy was " << energy << " but should have been " << expectedEnergy;
    throw std::runtime_error(ss.str());
  }
}
