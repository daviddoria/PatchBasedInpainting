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
#include "itkTimeProbe.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

#include "Initializers/InitializeFromMaskImage.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "DifferenceFunctions/SumSquaredPixelDifference.hpp"

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    std::cerr << "Required arguments: image" << std::endl;
    return EXIT_FAILURE;
  }
  itk::TimeProbe clock;

  clock.Start();

  std::string imageFilename = argv[1];
  std::cout << "Reading image: " << imageFilename << std::endl;

//  typedef itk::VectorImage<float, 2> ImageType;
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType* image = imageReader->GetOutput();

  std::cout << "Read image " << imageReader->GetOutput()->GetLargestPossibleRegion() << std::endl;

  // Create a fully valid mask
  Mask::Pointer maskData = Mask::New();
  maskData->SetRegions(image->GetLargestPossibleRegion());
  maskData->Allocate();
  maskData->FillBuffer(maskData->GetValidValue());

  Mask* mask = maskData.GetPointer();

  itk::ImageRegion<2> fullRegion = mask->GetLargestPossibleRegion();

  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;

  // Intialize all nodes
  itk::ImageRegionConstIteratorWithIndex<Mask>
      imageIterator(mask, fullRegion);

  unsigned int patchRadius = 10;

  typedef std::vector<ImagePatchPixelDescriptorType> PatchContainerType;
  PatchContainerType patches;

  while(!imageIterator.IsAtEnd())
  {
    itk::ImageRegion<2> region =
        ITKHelpers::GetRegionInRadiusAroundPixel(imageIterator.GetIndex(),
                                                 patchRadius);
    if(fullRegion.IsInside(region))
    {
      ImagePatchPixelDescriptorType descriptor(image, mask, region);
      descriptor.SetStatus(ImagePatchPixelDescriptorType::SOURCE_NODE);
      descriptor.SetFullyValid(true);
      descriptor.SetInsideImage(true);

      patches.push_back(descriptor);
    }
    ++imageIterator;
  }

  clock.Stop();
  std::cout << "Created " << patches.size() << " patches in: " << clock.GetTotal() << std::endl;
  clock.Start();

  // SSD (takes about the same time as SAD)
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType,
      SumSquaredPixelDifference<typename ImageType::PixelType> > PatchDifferenceType;
  PatchDifferenceType patchDifferenceFunctor;

  ImagePatchPixelDescriptorType queryPatch = patches[20];
  itk::ImageRegionConstIteratorWithIndex<Mask>
      patchIterator(mask, queryPatch.GetRegion());

  std::vector<itk::Offset<2> > offsets;
  while(!patchIterator.IsAtEnd())
  {
    offsets.push_back(ITKHelpers::IndexToOffset(patchIterator.GetIndex()));
    ++patchIterator;
  }

  queryPatch.SetValidOffsets(offsets);

  ImagePatchPixelDescriptorType result;

  // Iterate through all of the input elements
  float d_best = std::numeric_limits<float>::max();

  int a,b;
  for(PatchContainerType::const_iterator current = patches.begin();
      current != patches.end(); ++current)
  {
//    std::cout << current - patches.begin() << " ";
    b = 3;
    float d = patchDifferenceFunctor(*current, queryPatch);
    a = 3;

    if(d < d_best)
    {
      d_best = d;
      result = *current;
    }
  }
  std::cout << a << b << std::endl; // Prevent 'a' from being optimized out
  std::cout << "Found source: " << result.GetRegion() << std::endl;

  clock.Stop();
  std::cout << "Found source in: " << clock.GetTotal() << std::endl;

  return EXIT_SUCCESS;
}

