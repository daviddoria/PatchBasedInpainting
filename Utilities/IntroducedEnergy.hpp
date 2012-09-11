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
float IntroducedEnergy<TImage>::ComputeIntroducedEnergyPatchBoundary(const TImage* const inputImage, const Mask* const mask,
                                                                     itk::ImageRegion<2> sourceRegion, itk::ImageRegion<2> targetRegion)
{
  // Crop the source and target region so that the target region is inside the image and the source region has the same size/position crop
  sourceRegion = ITKHelpers::CropRegionAtPosition(sourceRegion, inputImage->GetLargestPossibleRegion(), targetRegion);

  targetRegion.Crop(inputImage->GetLargestPossibleRegion());

  // Create the magnitude image so we can compute gradients of a scalar image.
  typedef itk::Image<float, 2> MagnitudeImageType;
  MagnitudeImageType::Pointer magnitudeImage = MagnitudeImageType::New();
  ITKHelpers::MagnitudeImage(inputImage, magnitudeImage.GetPointer());

  // Call the data to operate on 'image' so the rest of this function
  // syntatically seems like it is operating on the input image directly.
  MagnitudeImageType* image = magnitudeImage;

  typedef itk::Image<itk::CovariantVector<float, 2> > GradientImageType;

  // Compute gradients in the target region
  GradientImageType::Pointer targetRegionGradientImage = GradientImageType::New();
  targetRegionGradientImage->SetRegions(image->GetLargestPossibleRegion());
  targetRegionGradientImage->Allocate();

  Derivatives::MaskedGradientInRegion(image, mask, targetRegion, targetRegionGradientImage.GetPointer());

  // Copy the source patch to the target patch
  MagnitudeImageType::Pointer inpaintedImage = MagnitudeImageType::New();
  ITKHelpers::DeepCopy(image, inpaintedImage.GetPointer());
  ITKHelpers::CopyRegion(image, inpaintedImage.GetPointer(), sourceRegion, targetRegion);

  // Compute the gradient of the inpainted image
  GradientImageType::Pointer inpaintedGradientImage = GradientImageType::New();
  inpaintedGradientImage->SetRegions(image->GetLargestPossibleRegion());
  inpaintedGradientImage->Allocate();

  Derivatives::MaskedGradientInRegion(inpaintedImage.GetPointer(), mask, targetRegion, inpaintedGradientImage.GetPointer());

  // Get the pixels on the boundary (outline) of the region
  typedef std::vector<itk::Index<2> > PixelContainer;
  PixelContainer patchBoundaryPixels = ITKHelpers::GetBoundaryPixels(targetRegion);

  // Remove boundary pixels that are not in the valid region of the patch (i.e. remove hole pixels)
  patchBoundaryPixels.erase(std::remove_if(patchBoundaryPixels.begin(), patchBoundaryPixels.end(),
                    [mask](const itk::Index<2>& queryPixel)
                    {
                      return mask->IsHole(queryPixel);
                    }),
                    patchBoundaryPixels.end());

  // Compare the gradient magnitude before and after the inpainting
  float gradientMagnitudeChange = 0.0f;

  for(PixelContainer::const_iterator boundaryPixelIterator = patchBoundaryPixels.begin();
      boundaryPixelIterator != patchBoundaryPixels.end(); ++boundaryPixelIterator)
  {
    gradientMagnitudeChange += (targetRegionGradientImage->GetPixel(*boundaryPixelIterator) -
                                inpaintedGradientImage->GetPixel(*boundaryPixelIterator)).GetSquaredNorm();
  }

//  std::cout << "ComputeIntroducedEnergyPatchBoundary: " << gradientMagnitudeChange << std::endl;
//  return gradientMagnitudeChange;

  if(this->GetDebugImages())
  {
    ITKHelpers::WriteImage(targetRegionGradientImage.GetPointer(), "PatchBoundaryEnergy_TargetGradient.mha");
    ITKHelpers::WriteImage(inpaintedGradientImage.GetPointer(), "PatchBoundaryEnergy_InpaintedGradient.mha");
    ITKHelpers::WriteImage(inpaintedImage.GetPointer(), "PatchBoundaryEnergy_Inpainted.mha");
  }

  if(patchBoundaryPixels.size() > 0)
  {
    float normalized = gradientMagnitudeChange / static_cast<float>(patchBoundaryPixels.size());
    return normalized;
  }
  else
  {
    std::cout << "ComputeIntroducedEnergyPatchBoundary had 0 pixels to use!" << std::endl;
    return 0;
  }
}

template <typename TImage>
float IntroducedEnergy<TImage>::ComputeIntroducedEnergyMaskBoundary(const TImage* const inputImage, const Mask* const mask,
                                                                    itk::ImageRegion<2> sourceRegion, itk::ImageRegion<2> targetRegion)
{
  // Crop the source and target region so that the target region is inside the image and the source region has the same size/position crop
  itk::Offset<2> offset = targetRegion.GetIndex() - sourceRegion.GetIndex();

  targetRegion.Crop(inputImage->GetLargestPossibleRegion());

  sourceRegion.SetIndex(sourceRegion.GetIndex() + offset);
  sourceRegion.Crop(inputImage->GetLargestPossibleRegion());
  sourceRegion.SetIndex(sourceRegion.GetIndex() - offset);

  // Create the magnitude image so we can compute gradients of a scalar image.
  typedef itk::Image<float, 2> MagnitudeImageType;
  MagnitudeImageType::Pointer magnitudeImage = MagnitudeImageType::New();
  ITKHelpers::MagnitudeImage(inputImage, magnitudeImage.GetPointer());

  // Call the data to operate on 'image' so the rest of this function
  // syntatically seems like it is operating on the input image directly.
  MagnitudeImageType* image = magnitudeImage;

  typedef itk::Image<itk::CovariantVector<float, 2> > GradientImageType;
  // Find pixels in the region and on the mask boundary
  typedef std::vector<itk::Index<2> > PixelContainer;
  PixelContainer holeBoundaryPixels = mask->FindBoundaryPixelsInRegion(targetRegion, Mask::HOLE);
  PixelContainer validBoundaryPixels = mask->FindBoundaryPixelsInRegion(targetRegion, Mask::VALID);
  PixelContainer boundaryPixels;
  boundaryPixels.insert(boundaryPixels.end(), holeBoundaryPixels.begin(), holeBoundaryPixels.end());
  boundaryPixels.insert(boundaryPixels.end(), validBoundaryPixels.begin(), validBoundaryPixels.end());

//  std::cout << "There are " << boundaryPixels.size() << " boundary pixels." << std::endl;

  // Compute gradients in the target region
  GradientImageType::Pointer targetRegionGradientImage = GradientImageType::New();
  targetRegionGradientImage->SetRegions(image->GetLargestPossibleRegion());
  targetRegionGradientImage->Allocate();

  Derivatives::MaskedGradientInRegion(image, mask, targetRegion, targetRegionGradientImage.GetPointer());

  // Copy the patch into an image (we copy the patch in the magnitude image because there is no
  // reason to copy it in the vector image and then take the magnitude again
  MagnitudeImageType::Pointer inpaintedImage = MagnitudeImageType::New();
  ITKHelpers::DeepCopy(image, inpaintedImage.GetPointer());
  ITKHelpers::CopyRegion(image, inpaintedImage.GetPointer(), sourceRegion, targetRegion);

  // Compute the gradient of the inpainted image
  GradientImageType::Pointer inpaintedGradientImage = GradientImageType::New();
  inpaintedGradientImage->SetRegions(image->GetLargestPossibleRegion());
  inpaintedGradientImage->Allocate();

  Derivatives::MaskedGradientInRegion(inpaintedImage.GetPointer(), mask, targetRegion, inpaintedGradientImage.GetPointer());

  // Compare the gradient magnitude before and after the inpainting
  float gradientMagnitudeChange = 0.0f;

  for(PixelContainer::const_iterator boundaryPixelIterator = boundaryPixels.begin();
      boundaryPixelIterator != boundaryPixels.end(); ++boundaryPixelIterator)
  {
    gradientMagnitudeChange += (targetRegionGradientImage->GetPixel(*boundaryPixelIterator) -
                                inpaintedGradientImage->GetPixel(*boundaryPixelIterator)).GetSquaredNorm();
  }

//  std::cout << "ComputeIntroducedEnergyMaskBoundary: " << gradientMagnitudeChange << std::endl;

  if(this->GetDebugImages())
  {
    ITKHelpers::WriteImage(targetRegionGradientImage.GetPointer(), "MaskBoundaryEnergy_TargetGradient.mha");
    ITKHelpers::WriteImage(inpaintedGradientImage.GetPointer(), "MaskBoundaryEnergy_InpaintedGradient.mha");
    ITKHelpers::WriteImage(inpaintedImage.GetPointer(), "MaskBoundaryEnergy_Inpainted.mha");
  }

  if(boundaryPixels.size() > 0)
  {
    float normalized = gradientMagnitudeChange / static_cast<float>(boundaryPixels.size());
    return normalized;
  }
  else
  {
    std::cout << "ComputeIntroducedEnergyMaskBoundary had 0 pixels to use!" << std::endl;
    return 0;
  }
//  return gradientMagnitudeChange;
}

template <typename TImage>
float IntroducedEnergy<TImage>::ComputeIntroducedEnergy(const TImage* const image, const Mask* const mask,
                                                        itk::ImageRegion<2> sourceRegion, itk::ImageRegion<2> targetRegion)
{
  return ComputeIntroducedEnergyPatchBoundary(image, mask, sourceRegion, targetRegion) +
      ComputeIntroducedEnergyMaskBoundary(image, mask, sourceRegion, targetRegion);
}

#endif
