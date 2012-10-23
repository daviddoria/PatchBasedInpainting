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

template <typename TPatchContainer, typename TPatch>
TPatch Naive(const TPatchContainer& patches, const TPatch& queryPatch);

template <typename TPatchContainer, typename TPatch>
TPatch Prefetch(const TPatchContainer& patches, const TPatch& queryPatch);

template <typename TPatchContainer, typename TPatch>
TPatch PreExtract(const TPatchContainer& patches, const TPatch& queryPatch);

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

  // Get one of the patches to use as the query (20 is arbitrary)
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

//  ImagePatchPixelDescriptorType result = Naive(patches, queryPatch);

//  ImagePatchPixelDescriptorType result = Prefetch(patches, queryPatch);

  ImagePatchPixelDescriptorType result = PreExtract(patches, queryPatch);

  std::cout << "Found source: " << result.GetRegion() << std::endl;

  clock.Stop();
  std::cout << "Found source in: " << clock.GetTotal() << std::endl;

  return EXIT_SUCCESS;
}

template <typename TPatchContainer, typename TPatch>
TPatch Naive(const TPatchContainer& patches, const TPatch& queryPatch)
{
  typedef ImagePatchDifference<TPatch,
      SumSquaredPixelDifference<typename TPatch::ImageType::PixelType> > PatchDifferenceType;
  PatchDifferenceType patchDifferenceFunctor;

  TPatch result;

  // Iterate through all of the input elements
  float d_best = std::numeric_limits<float>::max();

  for(typename TPatchContainer::const_iterator current = patches.begin();
      current != patches.end(); ++current)
  {
//    std::cout << current - patches.begin() << " ";

    float d = patchDifferenceFunctor(*current, queryPatch);

    if(d < d_best)
    {
      d_best = d;
      result = *current;
    }
  }

  return result;
}

template <typename TPatchContainer, typename TPatch>
TPatch Prefetch(const TPatchContainer& patches, const TPatch& queryPatch)
{
  typedef ImagePatchDifference<TPatch,
      SumSquaredPixelDifference<typename TPatch::ImageType::PixelType> > PatchDifferenceType;
  PatchDifferenceType patchDifferenceFunctor;

  TPatch result;

  // Iterate through all of the input elements
  float d_best = std::numeric_limits<float>::max();

  typename TPatch::ImageType::SizeValueType height = queryPatch.GetRegion().GetSize()[1];

  typename TPatch::ImageType* image = queryPatch.GetImage();

  // Prefetch the first pixel of each row in the query patch
  for(unsigned int row = queryPatch.GetRegion().GetIndex()[0]; row < height; ++row)
  {
    itk::Index<2> firstPixelInRow = {{static_cast<itk::Index<2>::IndexValueType>(queryPatch.GetRegion().GetIndex()[0]),
                                      static_cast<itk::Index<2>::IndexValueType>(queryPatch.GetRegion().GetIndex()[1] + row)}};
    __builtin_prefetch (&image->GetPixel(firstPixelInRow));
  }

  for(typename TPatchContainer::const_iterator current = patches.begin();
      current != patches.end(); ++current)
  {
//    std::cout << current - patches.begin() << " ";

    // Prefetch the first pixel of each row in the current patch
    for(unsigned int row = current->GetRegion().GetIndex()[0]; row < height; ++row)
    {
      itk::Index<2> firstPixelInRow = {{static_cast<itk::Index<2>::IndexValueType>(current->GetRegion().GetIndex()[0]),
                                        static_cast<itk::Index<2>::IndexValueType>(current->GetRegion().GetIndex()[1] + row)}};
      __builtin_prefetch (&image->GetPixel(firstPixelInRow));
    }

    float d = patchDifferenceFunctor(*current, queryPatch);

    if(d < d_best)
    {
      d_best = d;
      result = *current;
    }
  }

  return result;
}

template <typename TPatchContainer, typename TPatch>
TPatch PreExtract(const TPatchContainer& patches, const TPatch& queryPatch)
{
  typedef ImagePatchDifference<TPatch,
      SumSquaredPixelDifference<typename TPatch::ImageType::PixelType> > PatchDifferenceType;
  PatchDifferenceType patchDifferenceFunctor;

  // Extract the pixel values of the target patch, since these will be used in every patch comparison
  typename TPatch::ImageType* image = queryPatch.GetImage();

  typedef std::vector<itk::Offset<2> > OffsetVectorType;
  const OffsetVectorType* validOffsets = queryPatch.GetValidOffsetsAddress();

  typedef std::vector<typename TPatch::ImageType::PixelType> PixelVector;

  PixelVector targetPixels(validOffsets->size());

  for(OffsetVectorType::const_iterator offsetIterator = validOffsets->begin();
      offsetIterator < validOffsets->end(); ++offsetIterator)
  {
    itk::Offset<2> currentOffset = *offsetIterator;

    targetPixels[offsetIterator - validOffsets->begin()] = image->GetPixel(queryPatch.GetCorner() + currentOffset);
  }

  // Perform the differences
  TPatch result;

  // Iterate through all of the input elements
  float d_best = std::numeric_limits<float>::max();

  for(typename TPatchContainer::const_iterator current = patches.begin();
      current != patches.end(); ++current)
  {
//    std::cout << current - patches.begin() << " ";

    float d = patchDifferenceFunctor(*current, queryPatch, targetPixels);

    if(d < d_best)
    {
      d_best = d;
      result = *current;
    }
  }

  return result;
}
