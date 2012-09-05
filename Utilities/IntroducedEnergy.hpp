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

#ifndef IntroducedEnergy_HPP
#define IntroducedEnergy_HPP

#include "IntroducedEnergy.h"

#include "ImageProcessing/Derivatives.h"

template <typename TImage>
float IntroducedEnergy<TImage>::ComputeIntroducedEnergyPatchBoundary(const TImage* const image, const Mask* const mask,
                                                             const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
{
  // Compute gradients in the target region
  typedef itk::Image<itk::CovariantVector<float, 2> > GradientImageType;
  GradientImageType::Pointer targetRegionGradientImage = GradientImageType::New();
  targetRegionGradientImage->SetRegions(image->GetLargestPossibleRegion());
  targetRegionGradientImage->Allocate();

  Derivatives::MaskedGradientInRegion(image, mask, targetRegion, targetRegionGradientImage.GetPointer());

  // Compute gradients in the source region
  typedef itk::Image<itk::CovariantVector<float, 2> > GradientImageType;
  GradientImageType::Pointer sourceRegionGradientImage = GradientImageType::New();
  sourceRegionGradientImage->SetRegions(image->GetLargestPossibleRegion());
  sourceRegionGradientImage->Allocate();

  Derivatives::MaskedGradientInRegion(image, mask, sourceRegion, sourceRegionGradientImage.GetPointer());
}

template <typename TImage>
float IntroducedEnergy<TImage>::ComputeIntroducedEnergyMaskBoundary(const TImage* const image, const Mask* const mask,
                                                            const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
{
  // Find pixels in the region and on the mask boundary
  std::vector<itk::Index<2> > boundaryPixels = mask->FindBoundaryPixelsInRegion(targetRegion);

  // Copy the patch into an image
  typename TImage::Pointer filledImage = TImage::New();
  ITKHelpers::DeepCopyInRegion(image, targetRegion, filledImage.GetPointer());
}

template <typename TImage>
float IntroducedEnergy<TImage>::ComputeIntroducedEnergy(const TImage* const image, const Mask* const mask,
                                                const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
{
  return ComputeIntroducedEnergyPatchBoundary(image, mask, sourceRegion, targetRegion) +
      ComputeIntroducedEnergyMaskBoundary(image, mask, sourceRegion, targetRegion);
}

#endif
