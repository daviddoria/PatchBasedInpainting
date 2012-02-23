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

#include "Helpers/ITKHelpers.h"

#include <iostream>
#include <stdexcept>

static void DemoExtractRegion();
static void DemoXORRegions();

int main()
{
  DemoExtractRegion();
  DemoXORRegions();
  return EXIT_SUCCESS;
}

void DemoExtractRegion()
{
  std::cout << "DemoExtractRegion()" << std::endl;
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{11,11}};
  itk::ImageRegion<2> imageRegion(imageCorner, imageSize);

  typedef itk::Image<unsigned char, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(imageRegion);
  image->Allocate();
  image->FillBuffer(0);

  itk::ImageRegionIterator<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < static_cast<unsigned int>(imageSize[0]/2))
      {
      imageIterator.Set(0);
      }
    else
      {
      imageIterator.Set(255);
      }
    ++imageIterator;
    }


  itk::Size<2> regionSize = {{3,3}};
  itk::Index<2> regionCorner = {{imageSize[0]/2 - regionSize[0]/2,
                                 imageSize[1]/2 - regionSize[1]/2}};
  
  itk::ImageRegion<2> region(regionCorner, regionSize);
  std::cout << "region to extract: " << region << std::endl;

  // Dilate the hole
  ImageType::Pointer extractedImage = ImageType::New();
  ITKHelpers::ExtractRegion(image.GetPointer(), region, extractedImage.GetPointer());

  ITKHelpers::PrintImage(extractedImage.GetPointer());
}

void DemoXORRegions()
{
  std::cout << "DemoXORRegions()" << std::endl;
  
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{11,11}};
  itk::ImageRegion<2> imageRegion(imageCorner, imageSize);

  typedef itk::Image<unsigned char, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(imageRegion);
  image->Allocate();
  image->FillBuffer(0);

  itk::ImageRegionIterator<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < static_cast<unsigned int>(imageSize[0]/2))
      {
      imageIterator.Set(0);
      }
    else
      {
      imageIterator.Set(255);
      }
    ++imageIterator;
    }


  itk::Size<2> regionSize = {{3,3}};
  
  itk::Index<2> region1Corner = {{imageSize[0]/2 - regionSize[0]/2,
                                 imageSize[1]/2 - regionSize[1]/2}};

  itk::ImageRegion<2> region1(region1Corner, regionSize);
  std::cout << "region1: " << region1 << std::endl;

  ITKHelpers::PrintRegion(image.GetPointer(), region1);
  
  itk::Index<2> region2Corner = {{imageSize[0]/2 - regionSize[0]/2,
                                 imageSize[1]/2 - regionSize[1]/2 + 1}};

  itk::ImageRegion<2> region2(region2Corner, regionSize);
  std::cout << "region2: " << region2 << std::endl;
  ITKHelpers::PrintRegion(image.GetPointer(), region2);

//   ITKHelpers::XORRegions(image, const itk::ImageRegion<2>& region1, const TImage* const image2, const itk::ImageRegion<2>& region2, itk::Image<bool, 2>* const output);
// 
//   ITKHelpers::PrintImage(extractedImage.GetPointer());
}
