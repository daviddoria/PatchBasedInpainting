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

#include "itkNeighborhoodAlgorithm.h"
#include "itkAndImageFilter.h"
#include "itkNumericTraits.h"

#include <iomanip> // setfill, setw

/////////////// Declarations ///////////////////////
template <typename TImage, typename TMask>
itk::Index<2> SelfPatchMatch(TImage* image, TMask* mask, itk::Index<2> queryPixel, unsigned int patchRadius, unsigned int iteration);

template< typename TImage, typename TMask>
void ComputeDifferenceImage(const TImage* image, const TMask* mask,
                            itk::Index<2> queryPixel, unsigned int patchRadius, FloatImageType* output);

template< typename TImage, typename TMask>
float PatchDifference(const TImage* image, const TMask* mask, itk::Index<2> queryPixel, itk::Index<2> currentPixel, unsigned int patchRadius, float RGBWeight);

/////////////// Definitions ///////////////////////

// This is the main driver function. It calls PatchImageDifference and then finds the best match in the area we want to match from and returns it.
// This was written for an inpainting algorithm - so the 'mask' should be interpreted as the region to inpaint
// (i.e. no data was defined in the 'image' in the 'mask != 0' region.)
// We want the best matching patch to come from a region that has no 'missing' pixels - i.e. it does not overlap the non-zero region of the mask.
template <typename TImage, typename TMask>
itk::Index<2> SelfPatchMatch(TImage* image, TMask* mask, itk::Index<2> queryPixel, unsigned int patchRadius, unsigned int iteration)
{
  FloatImageType::Pointer differenceImage = FloatImageType::New();

  ComputeDifferenceImage(image, mask, queryPixel, patchRadius, differenceImage);

  // Compute the maximum value of the difference image. We will use this to fill the constant blank patch
  //float highestValue = MaxValue<FloatImageType>(differenceImage);
  float highestValue = itk::NumericTraits< typename FloatImageType::PixelType >::max();

  // Paste a blank patch into the correlation image so that we don't match a region within the current patch or anywhere near it
  FloatImageType::Pointer blankPatch = FloatImageType::New();
  CreateConstantPatch<FloatImageType>(blankPatch, highestValue, patchRadius * 2);
  CopyPatchIntoImage<FloatImageType>(blankPatch, differenceImage, queryPixel);

  // Set the differenceImage pixels in the masked region to a very high value (we don't want to match here)
  itk::ImageRegionIterator<FloatImageType> differenceImageIterator(differenceImage, differenceImage->GetLargestPossibleRegion());
  differenceImageIterator.GoToBegin();

  itk::ImageRegionConstIterator<TMask> maskIterator(mask, mask->GetLargestPossibleRegion());
  maskIterator.GoToBegin();

  while(!differenceImageIterator.IsAtEnd())
    {
    if(maskIterator.Get() != 0)
      {
      differenceImageIterator.Set(highestValue);
      }
    ++differenceImageIterator;
    ++maskIterator;
    }

  std::stringstream padded;
  padded << "Difference_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  WriteImage<FloatImageType>(differenceImage, padded.str());

  itk::Index<2> bestMatchIndex = MinValueLocation<FloatImageType>(differenceImage);
  //std::cout << "BestMatchIndex: " << bestMatchIndex << std::endl;
  return bestMatchIndex;
}

// This function computes the average difference of the corresponding pixels in a region around 'queryPixel' and 'currentPixel' where 'mask' is non-zero. We require all pixels in region 2 to be valid, so that we guarantee when the patch is copied the region will be filled in with valid pixels.
template< typename TImage, typename TMask>
float PatchDifference(const TImage* image, const TMask* mask, itk::Index<2> queryPixel, itk::Index<2> currentPixel, unsigned int patchRadius, float RGBWeight)
{
  itk::ImageRegionConstIterator<TImage> queryPatchIterator(image, GetRegionInRadiusAroundPixel(queryPixel, patchRadius));
  queryPatchIterator.GoToBegin();

  itk::ImageRegionConstIterator<TImage> currentPatchIterator(image, GetRegionInRadiusAroundPixel(currentPixel, patchRadius));
  currentPatchIterator.GoToBegin();

  itk::ImageRegionConstIterator<TMask> queryMaskIterator(mask, GetRegionInRadiusAroundPixel(queryPixel, patchRadius));
  queryMaskIterator.GoToBegin();

  itk::ImageRegionConstIterator<TMask> currentMaskIterator(mask, GetRegionInRadiusAroundPixel(currentPixel, patchRadius));
  currentMaskIterator.GoToBegin();

  typename TImage::PixelType queryPatchPixel;
  typename TImage::PixelType currentPatchPixel;
  typename TMask::PixelType queryMaskPixel;
  typename TMask::PixelType currentMaskPixel;

  double totalSquaredDifference = 0;
  unsigned int numberOfValidPixels = 0;
  while(!queryPatchIterator.IsAtEnd())
    {
    queryMaskPixel = queryMaskIterator.Get();
    currentMaskPixel = currentMaskIterator.Get();
    if(currentMaskPixel != 0) // part of the current patch in the hole, but we require the best patch to be entirely outside the hole
      {
      return itk::NumericTraits< typename FloatImageType::PixelType >::max();
      }
    if(queryMaskPixel != 0) // If these are '!=' or '==' depends on if the mask is "black = valid" or "black = invalid"
      {
      // Do nothing
      }
    else
      {
      queryPatchPixel = queryPatchIterator.Get();
      currentPatchPixel = currentPatchIterator.Get();

      // Weight the RGB components
      for(unsigned int component = 0; component < 3; component++)
        {
        float pixel1 = static_cast<float>(queryPatchPixel[component]);
        float pixel2 = static_cast<float>(currentPatchPixel[component]);
        totalSquaredDifference += (0.33 * RGBWeight) * ( (pixel1 - pixel2) * (pixel1 - pixel2) );
        }

      // Weight the remaining components
      for(unsigned int component = 3; component < TImage::PixelType::GetNumberOfComponents(); component++)
        {
        float pixel1 = static_cast<float>(queryPatchPixel[component]);
        float pixel2 = static_cast<float>(currentPatchPixel[component]);
        totalSquaredDifference += 1./(TImage::PixelType::GetNumberOfComponents() - 3.) * (1. - RGBWeight) * ( (pixel1 - pixel2) * (pixel1 - pixel2) );
        }
      numberOfValidPixels++;
      }

    ++queryPatchIterator;
    ++currentPatchIterator;
    ++queryMaskIterator;
    ++currentMaskIterator;
    }

  //return totalSquaredDifference/static_cast<float>(numberOfValidPixels); // average squared difference
  return totalSquaredDifference; // sum of squared differences
}

template< typename TImage, typename TMask>
void ComputeDifferenceImage(const TImage* image, const TMask* mask,
                            itk::Index<2> queryPixel, unsigned int patchRadius, FloatImageType* output)
{
  // This function moves a patch centered at 'queryPixel' over an image and computes the mean difference of the valid pixels at each point.
  // Only patch with zero invalid (masked) pixels are compared.

  // Create an output image the same size as the input image
  output->SetRegions(image->GetLargestPossibleRegion());
  output->Allocate();

  typedef itk::ImageRegionConstIterator<TImage> ConstIteratorType;
  typedef itk::ImageRegionIterator<TImage> IteratorType;
  typedef itk::ImageRegionIterator<FloatImageType> OutputIteratorType;

  typedef itk::NeighborhoodAlgorithm
    ::ImageBoundaryFacesCalculator< TImage> FaceCalculatorType;

  FaceCalculatorType faceCalculator;

  typename FaceCalculatorType::FaceListType faceList;
  faceList = faceCalculator(image, image->GetLargestPossibleRegion(), GetRegionInRadiusAroundPixel(queryPixel, patchRadius).GetSize());

  typename FaceCalculatorType::FaceListType::iterator faceListIterator = faceList.begin();

  itk::ImageRegionConstIteratorWithIndex<TImage> centralIterator(image,*faceListIterator);
  OutputIteratorType outputIterator(output,*faceListIterator);
  centralIterator.GoToBegin();
  outputIterator.GoToBegin();

  while(!centralIterator.IsAtEnd())
    {
    float difference = PatchDifference(image, mask, queryPixel, centralIterator.GetIndex(), patchRadius);
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