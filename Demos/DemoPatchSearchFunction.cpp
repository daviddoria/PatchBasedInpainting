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

// ITK
#include "itkImageFileReader.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

#include "Initializers/InitializeFromMaskImage.hpp"

// Difference functions
#include "DifferenceFunctions/Patch/ImagePatchDifference.hpp"
#include "DifferenceFunctions/Pixel/SumSquaredPixelDifference.hpp"

int main(int argc, char *argv[])
{
  if(argc != 3)
  {
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
  }

  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

//  typedef itk::VectorImage<float, 2> ImageType;
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType* image = imageReader->GetOutput();

  std::cout << "Read image " << imageReader->GetOutput()->GetLargestPossibleRegion() << std::endl;

//  typedef itk::ImageFileReader<Mask> MaskReaderType;
//  MaskReaderType::Pointer maskReader = MaskReaderType::New();
//  maskReader->SetFileName(maskFilename.c_str());
//  maskReader->Update();

//  Mask* mask = maskReader->GetOutput();

//  std::cout << "Read mask " << maskReader->GetOutput()->GetLargestPossibleRegion() << std::endl;

  // Create a fully valid mask
  Mask::Pointer maskData = Mask::New();
  maskData->SetRegions(image->GetLargestPossibleRegion());
  maskData->Allocate();
  maskData->FillBuffer(maskData->GetValidValue());

  Mask* mask = maskData.GetPointer();

  itk::ImageRegion<2> fullRegion = mask->GetLargestPossibleRegion();

  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;

  // Intialize all nodes
  itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(mask,
                                                             fullRegion);

  unsigned int patchRadius = 10;

  typedef std::vector<ImagePatchPixelDescriptorType> PatchContainerType;
  PatchContainerType patches;

  while(!imageIterator.IsAtEnd())
  {
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(imageIterator.GetIndex(), patchRadius);
    if(image->GetLargestPossibleRegion().IsInside(region))
    {
      ImagePatchPixelDescriptorType descriptor(image, mask, region);
      patches.push_back(descriptor);
    }
    ++imageIterator;
  }

  // SSD (takes about the same time as SAD)
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      SumSquaredPixelDifference<typename ImageType::PixelType> > PatchDifferenceType;
  PatchDifferenceType patchDifferenceFunctor;

  ImagePatchPixelDescriptorType queryPatch = patches[0];
  ImagePatchPixelDescriptorType result;

  // Iterate through all of the input elements
  float d_best = std::numeric_limits<float>::max();

  for(PatchContainerType::const_iterator current = patches.begin();
      current != patches.end(); ++current)
  {
    float d = patchDifferenceFunctor(*current, queryPatch);

    if(d < d_best)
    {
      d_best = d;
      result = *current;
    }
  }
  std::cout << "Found source: " << result.GetRegion() << std::endl;

  return EXIT_SUCCESS;
}
