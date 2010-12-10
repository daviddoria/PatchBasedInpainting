/*=========================================================================
 *
 *  Copyright David Doria 2010 daviddoria@gmail.com
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

#ifndef SelfPatchMatch_H
#define SelfPatchMatch_H

#include "Helpers.h"
#include "Types.h"

#include "itkRegionOfInterestImageFilter.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkAndImageFilter.h"
#include "itkNumericTraits.h"

/////////////// Declarations ///////////////////////
template< typename TImage, typename TMask>
float ImageDifference(const TImage* image1, const TImage* image2, const TMask* validityMask);

template <typename TImage, typename TMask>
itk::Index<2> SelfPatchMatch(TImage* image, TMask* mask, itk::Index<2> queryPixel, unsigned int patchRadius);

template< typename TImage, typename TMask>
void PatchImageDifference(const TImage* image, const TMask* imageMask, const TImage* patch, FloatImageType* output);

/////////////// Definitions ///////////////////////
template< typename TImage, typename TMask>
float ImageDifference(const TImage* image1, const TImage* image2, const TMask* validityMask)
{
  // This function computes the average difference of the corresponding pixels in 'image1' and 'image2' where 'validityMask' is non-zero.
  // 'image1', 'image2', and 'validityMask' must be the same size.

  itk::ImageRegionConstIterator<TImage> image1Iterator(image1, image1->GetLargestPossibleRegion());
  image1Iterator.GoToBegin();

  itk::ImageRegionConstIterator<TImage> image2Iterator(image2, image2->GetLargestPossibleRegion());
  image2Iterator.GoToBegin();

  itk::ImageRegionConstIterator<TMask> maskIterator(validityMask, validityMask->GetLargestPossibleRegion());
  maskIterator.GoToBegin();

  typename TImage::PixelType image1Pixel;
  typename TImage::PixelType image2Pixel;
  typename TMask::PixelType maskPixel;

  double totalSquaredDifference = 0;
  unsigned int numberOfValidPixels = 0;
  while(!image1Iterator.IsAtEnd())
    {
    maskPixel = maskIterator.Get();
    if(maskPixel != 0) // The depends on if the mask if "black = valid" or "black = invalid"
      {
      ++image1Iterator;
      ++image2Iterator;
      ++maskIterator;
      continue;
      }

    image1Pixel = image1Iterator.Get();
    image2Pixel = image2Iterator.Get();

    for(unsigned int component = 0; component < TImage::PixelType::GetNumberOfComponents(); component++)
      {
      float pixel1 = static_cast<float>(image1Pixel[component]);
      float pixel2 = static_cast<float>(image2Pixel[component]);
      totalSquaredDifference += ( (pixel1 - pixel2) * (pixel1 - pixel2) );
      }
    numberOfValidPixels++;

    ++image1Iterator;
    ++image2Iterator;
    ++maskIterator;
    }

  //return totalSquaredDifference/static_cast<float>(numberOfValidPixels); // average squared difference
  return totalSquaredDifference; // sum of squared differences
}

// This function takes an image, a mask, a query pixel, and a patchRadius and finds the best pixel patch.
// This was written for an inpainting algorithm - so the 'mask' should be interpreted as the region to inpaint
// (i.e. no data was defined in the 'image' in the 'mask != 0' region.)
// We want the best matching patch to come from a region that has no 'missing' pixels - i.e. it does not overlap the non-zero region of the mask.
template <typename TImage, typename TMask>
itk::Index<2> SelfPatchMatch(TImage* image, TMask* mask, itk::Index<2> queryPixel, unsigned int patchRadius)
{
  typedef itk::RegionOfInterestImageFilter< TImage,
                                            TImage> ExtractImageFilterType;
  typename ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
  extractImageFilter->SetRegionOfInterest(GetRegionInRadiusAroundPixel(queryPixel, patchRadius));
  extractImageFilter->SetInput(image);
  extractImageFilter->Update();

  FloatImageType::Pointer differenceImage = FloatImageType::New();

  PatchImageDifference(image, mask, extractImageFilter->GetOutput(), differenceImage);

  // Compute the maximum value of the difference image. We will use this to fill the constant blank patch
  float highestValue = MaxValue<FloatImageType>(differenceImage);

  // Paste a blank patch into the correlation image so that we don't match a region within the current patch or anywhere near it
  FloatImageType::Pointer blankPatch = FloatImageType::New();
  CreateConstantPatch<FloatImageType>(blankPatch, highestValue, patchRadius * 2);
  CopyPatchIntoImage<FloatImageType>(blankPatch, differenceImage, queryPixel);

  //WriteImage<FloatImageType>(differenceImage, "BlankedCorrelation.mhd");

  itk::Index<2> bestMatchIndex = MinValueLocation<FloatImageType>(differenceImage);
  //std::cout << "BestMatchIndex: " << bestMatchIndex << std::endl;
  return bestMatchIndex;
}


template< typename TImage, typename TMask>
void PatchImageDifference(const TImage* image, const TMask* imageMask, const TImage* patch, FloatImageType* output)
{
  // This function moves a patch over an image and computes the mean difference of the valid pixels at each point.
  // Only patch with zero invalid (masked) pixels are compared.

  // Preconditions
  if(image->GetLargestPossibleRegion() != imageMask->GetLargestPossibleRegion())
    {
    std::cerr << "image and imageMask must be the same size!" << std::endl;
    exit(-1);
    }

  if(patch->GetLargestPossibleRegion().GetSize()[0] != patch->GetLargestPossibleRegion().GetSize()[1])
    {
    std::cerr << "Patch must be square!" << std::endl;
    exit(-1);
    }

  // Create an output image the same size as the input image
  output->SetRegions(image->GetLargestPossibleRegion());
  output->Allocate();

  int patchRadius = patch->GetLargestPossibleRegion().GetSize()[0]/2;

  typedef itk::ImageRegionConstIterator<TImage> ConstIteratorType;
  typedef itk::ImageRegionIterator<TImage> IteratorType;
  typedef itk::ImageRegionIterator<FloatImageType> OutputIteratorType;

  typedef itk::NeighborhoodAlgorithm
    ::ImageBoundaryFacesCalculator< TImage> FaceCalculatorType;

  FaceCalculatorType faceCalculator;

  typename FaceCalculatorType::FaceListType faceList;
  faceList = faceCalculator(image, image->GetLargestPossibleRegion(), patch->GetLargestPossibleRegion().GetSize());

  typename FaceCalculatorType::FaceListType::iterator faceListIterator = faceList.begin();

  ConstIteratorType centralIterator(image,*faceListIterator);
  OutputIteratorType outputIterator(output,*faceListIterator);
  centralIterator.GoToBegin();
  outputIterator.GoToBegin();

  typedef itk::RegionOfInterestImageFilter<TImage,TImage> ExtractImageFilterType;
  typedef itk::RegionOfInterestImageFilter<TMask,TMask> ExtractMaskFilterType;

  while(!centralIterator.IsAtEnd())
    {
    // Get the patch of image around the current pixel
    typename ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
    extractImageFilter->SetRegionOfInterest(GetRegionInRadiusAroundPixel(centralIterator.GetIndex(), patchRadius));
    extractImageFilter->SetInput(image);
    extractImageFilter->Update();

    // Get the patch of mask around the current pixel
    typename ExtractMaskFilterType::Pointer extractMaskFilter = ExtractMaskFilterType::New();
    extractMaskFilter->SetRegionOfInterest(GetRegionInRadiusAroundPixel(centralIterator.GetIndex(), patchRadius));
    extractMaskFilter->SetInput(imageMask);
    extractMaskFilter->Update();

    // Check if the current pixel's surrounding mask has any invalid pixels. If it does, we don't want to compare it.
    if(MaxValue<TMask>(extractMaskFilter->GetOutput()) > 0) // there are some masked pixels
      {
      outputIterator.Set(itk::NumericTraits< typename FloatImageType::PixelType >::max()); // Set the distance very high (i.e. bad match)
      ++centralIterator;
      ++outputIterator;
      continue;
      }

    float difference = ImageDifference<TImage, TMask>(extractImageFilter->GetOutput(), patch, extractMaskFilter->GetOutput());
    outputIterator.Set(difference);

    ++centralIterator;
    ++outputIterator;
    }

  ++faceListIterator;
  // Iterate over all of the boundary regions setting their distance to a very high value

  while(faceListIterator != faceList.end())
    {
    OutputIteratorType outputFaceIterator(output,*faceListIterator);
    outputFaceIterator.GoToBegin();
    while(!outputFaceIterator.IsAtEnd())
      {
      outputFaceIterator.Set(itk::NumericTraits< typename FloatImageType::PixelType >::max());
      ++outputFaceIterator;
      }
    ++faceListIterator;
    }

}


#endif