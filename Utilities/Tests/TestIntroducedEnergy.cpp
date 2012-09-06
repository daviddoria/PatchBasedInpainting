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

// Submodules
#include <Mask/Mask.h>
#include <Helpers/Helpers.h>

// Custom
#include "IntroducedEnergy.h"
#include "Testing/Testing.h"

// STL
#include <stdexcept>

// ITK
#include "itkImage.h"

int main(int, char*[])
{
//  typedef itk::Image<itk::CovariantVector<int, 3>, 2> ImageType;
  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;

  ImageType::PixelType red;
  red.Fill(0);
  red[0] = 255;

  ImageType::PixelType black;
  black.Fill(0);

  ImageType::PixelType white;
  white.Fill(255);

  ImageType::PixelType green; // Note this is not 255 because then the magnitude of red and green would be the same,
  // which makes debugging hard since the gradient of the magnitude image is used internally (in IntroducedEnergy).
  green.Fill(0);
  green[1] = 122;

  ImageType::PixelType blue;
  blue.Fill(0);
  blue[2] = 255;

  ImageType::Pointer image = ImageType::New();
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{100,100}};

  itk::ImageRegion<2> region(imageCorner,imageSize);
  image->SetRegions(region);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(region);
  mask->Allocate();

  itk::ImageRegionIteratorWithIndex<Mask> initializeMaskIterator(mask, mask->GetLargestPossibleRegion());

  while(!initializeMaskIterator.IsAtEnd())
  {
    if(initializeMaskIterator.GetIndex()[0] < 55)
    {
      initializeMaskIterator.Set(mask->GetHoleValue());
    }
    else
    {
      initializeMaskIterator.Set(mask->GetValidValue());
    }

    ++initializeMaskIterator;
  }

  ITKHelpers::WriteImage(mask.GetPointer(), "mask.png");

  // Create a red image
  itk::ImageRegionIterator<ImageType> initializeIterator(image, image->GetLargestPossibleRegion());

  while(!initializeIterator.IsAtEnd())
  {
    initializeIterator.Set(red);

    ++initializeIterator;
  }

  // Setup source and target patch
  itk::Size<2> patchSize = {{10,10}};

  itk::Index<2> sourceCorner = {{10,10}};
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);

  itk::Index<2> targetCorner = {{50,50}};
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);

  itk::Index<2> perfectSourceCorner = {{75,75}};
  itk::ImageRegion<2> perfectSourceRegion(perfectSourceCorner, patchSize);

  // Make the source patch green
  itk::ImageRegionIterator<ImageType> sourceRegionIterator(image, sourceRegion);

  while(!sourceRegionIterator.IsAtEnd())
  {
    sourceRegionIterator.Set(green);

    ++sourceRegionIterator;
  }

  ITKHelpers::WriteImage(image.GetPointer(), "image.png");

  {
    ImageType::Pointer regionHighlightImage = ImageType::New();
    ITKHelpers::DeepCopy(image.GetPointer(), regionHighlightImage.GetPointer());

    ITKHelpers::OutlineRegion(regionHighlightImage.GetPointer(), sourceRegion, white);
    ITKHelpers::OutlineRegion(regionHighlightImage.GetPointer(), targetRegion, black);
    ITKHelpers::OutlineRegion(regionHighlightImage.GetPointer(), perfectSourceRegion, blue);

    ITKHelpers::WriteImage(regionHighlightImage.GetPointer(), "regions.png");
  }

  IntroducedEnergy<ImageType> introducedEnergy;
  introducedEnergy.SetDebugImages(true);

  // Bad match
  {
    std::cout << "Bad match:" << std::endl;

    float patchBoundaryEnergy = introducedEnergy.ComputeIntroducedEnergyPatchBoundary(image, mask, sourceRegion, targetRegion);
    std::cout << "patchBoundaryEnergy: " << patchBoundaryEnergy << std::endl;

    float maskBoundaryEnergy = introducedEnergy.ComputeIntroducedEnergyMaskBoundary(image, mask, sourceRegion, targetRegion);
    std::cout << "maskBoundaryEnergy: " << maskBoundaryEnergy << std::endl;

    float totalEnergy = introducedEnergy.ComputeIntroducedEnergy(image, mask, sourceRegion, targetRegion);
    std::cout << "totalEnergy: " << totalEnergy << std::endl;
  }

  // Perfect match
  {
    std::cout << "Perfect match:" << std::endl;
  //  IntroducedEnergy<ImageType> introducedEnergy;
    typedef IntroducedEnergy<ImageType> IntroducedEnergyType;

    float patchBoundaryEnergy = introducedEnergy.ComputeIntroducedEnergyPatchBoundary(image, mask, perfectSourceRegion, targetRegion);
    std::cout << "patchBoundaryEnergy: " << patchBoundaryEnergy << std::endl;

    float maskBoundaryEnergy = introducedEnergy.ComputeIntroducedEnergyMaskBoundary(image, mask, perfectSourceRegion, targetRegion);
    std::cout << "maskBoundaryEnergy: " << maskBoundaryEnergy << std::endl;

    float totalEnergy = introducedEnergy.ComputeIntroducedEnergy(image, mask, perfectSourceRegion, targetRegion);
    std::cout << "totalEnergy: " << totalEnergy << std::endl;
  }

  return EXIT_SUCCESS;
}
