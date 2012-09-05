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
  typedef itk::Image<itk::CovariantVector<int, 3>, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{100,100}};

  itk::ImageRegion<2> region(imageCorner,imageSize);
  image->SetRegions(region);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(region);
  mask->Allocate();
  mask->FillBuffer(mask->GetValidValue());

  // Create a red image
  itk::ImageRegionIterator<ImageType> initializeIterator(image, image->GetLargestPossibleRegion());

  ImageType::PixelType red;
  red.Fill(0);
  red[0] = 255;
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

  // Make the source patch green
  itk::ImageRegionIterator<ImageType> sourceRegionIterator(image, sourceRegion);

  ImageType::PixelType green;
  green.Fill(0);
  green[1] = 255;
  while(!sourceRegionIterator.IsAtEnd())
  {
    sourceRegionIterator.Set(green);

    ++sourceRegionIterator;
  }

//  IntroducedEnergy<ImageType> introducedEnergy;
  typedef IntroducedEnergy<ImageType> IntroducedEnergyType;

//  float patchBoundaryEnergy = IntroducedEnergyType::ComputeIntroducedEnergyPatchBoundary(image, mask, sourceRegion, targetRegion);
//  std::cout << "patchBoundaryEnergy: " << patchBoundaryEnergy << std::endl;

//  float maskBoundaryEnergy = IntroducedEnergyType::ComputeIntroducedEnergyMaskBoundary(image, mask, sourceRegion, targetRegion);
//  std::cout << "maskBoundaryEnergy: " << maskBoundaryEnergy << std::endl;

//  float totalEnergy = IntroducedEnergyType::ComputeIntroducedEnergy(image, mask, sourceRegion, targetRegion);
//  std::cout << "totalEnergy: " << totalEnergy << std::endl;

  return EXIT_SUCCESS;
}
