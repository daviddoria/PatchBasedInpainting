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

#ifndef Isophotes_HPP
#define Isophotes_HPP

#include "Isophotes.h"

// Custom
#include "Derivatives.h"
#include "Helpers/Helpers.h"
#include "Mask/MaskOperations.h"
#include "Utilities/RotateVectors.h"

// ITK
#include "itkRGBToLuminanceImageFilter.h"

template <typename TVectorImageType, typename TIsophoteImageType>
void Isophotes::ComputeColorIsophotesInRegion(const TVectorImageType* const image, const Mask* const mask,
                                   const itk::ImageRegion<2>& region , TIsophoteImageType* const isophotes)
{
  /*
   * 'isophotes' must already be initialized to the right size and allocated.
   */
  assert(isophotes);
  assert(isophotes->GetLargestPossibleRegion() == image->GetLargestPossibleRegion());
  
  itk::ImageRegion<2> fullRegion = mask->GetLargestPossibleRegion();

//  std::cout << "image region: " << image->GetLargestPossibleRegion()
//            << " mask region: " << mask->GetLargestPossibleRegion() << std::endl;
  assert(image->GetLargestPossibleRegion() == mask->GetLargestPossibleRegion());
  assert(image->GetLargestPossibleRegion().IsInside(region));

  std::cout << "ComputeColorIsophotesInRegion()" << std::endl;
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  std::cout << "VectorImageToRGBImageInRegion()" << std::endl;
  ITKHelpers::VectorImageToRGBImageInRegion(image, rgbImage, region);

  assert(image->GetLargestPossibleRegion() == rgbImage->GetLargestPossibleRegion());

  //HelpersOutput::WriteImageConditional<RGBImageType>(rgbImage, "Debug/Initialize.rgb.mha", this->DebugImages);
  std::cout << "LuminanceFilterType" << std::endl;
  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->GetOutput()->SetRequestedRegion(region);
  luminanceFilter->Update();

  std::cout << "Luminance image" << std::endl;
  FloatScalarImageType::Pointer luminanceImage = FloatScalarImageType::New();
  luminanceImage->SetRegions(fullRegion);
  luminanceImage->Allocate();
  ITKHelpers::DeepCopyInRegion(luminanceFilter->GetOutput(), region, luminanceImage.GetPointer());

  std::cout << "blur " << region << std::endl;
  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel. From TestIsophotes.cpp, it actually seems like not blurring,
  // but using a masked sobel operator produces the most reliable isophotes.
  unsigned int kernelRadius = 0;
  std::cout << "Before MaskedBlurInRegion" << std::endl;
  MaskOperations::MaskedBlurInRegion(luminanceImage.GetPointer(), mask, region,
                             kernelRadius, blurredLuminance.GetPointer());

  //HelpersOutput::WriteImageConditional(blurredLuminance,
  //                                  "Debug/Initialize.blurredLuminance.mha", true);

  //ITKHelpers::InitializeImage(isophotes, image->GetLargestPossibleRegion());
  std::cout << "ComputeMaskedIsophotesInRegion" << std::endl;
  Isophotes::ComputeMaskedIsophotesInRegion(blurredLuminance.GetPointer(), mask, region, isophotes);

//   if(this->DebugImages)
//     {
//     ITKHelpers::Write2DVectorImage(this->IsophoteImage, "Debug/Initialize.IsophoteImage.mha");
//     }
  //LeaveFunction("ComputeIsophotes()");
}

template <typename TVectorImageType, typename TIsophoteImageType>
void Isophotes::ComputeMaskedIsophotesInRegion(const TVectorImageType* const image, const Mask* const mask,
                                    const itk::ImageRegion<2>& region, TIsophoteImageType* const outputIsophotes)
{
  //Helpers::WriteImageConditional<FloatScalarImageType>(image, "Debug/ComputeMaskedIsophotes.luminance.mha",
//                                                      this->DebugImages);

  typename TIsophoteImageType::Pointer gradient = TIsophoteImageType::New();
  ITKHelpers::InitializeImage(gradient.GetPointer(), region);
  std::cout << "ComputeMaskedIsophotesInRegion: MaskedGradientInRegion" << std::endl;
  Derivatives::MaskedGradientInRegion(image, mask, region, gradient.GetPointer());

  //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(gradient,
  //               "Debug/ComputeMaskedIsophotes.gradient.mha", this->DebugImages);
  //Helpers::Write2DVectorImage(gradient, "Debug/ComputeMaskedIsophotes.gradient.mha");

  // Rotate the gradient 90 degrees to obtain isophotes from gradient
  typedef itk::UnaryFunctorImageFilter<TIsophoteImageType, TIsophoteImageType,
  RotateVectors<typename TIsophoteImageType::PixelType,
                typename TIsophoteImageType::PixelType> > FilterType;

  typename FilterType::Pointer rotateFilter = FilterType::New();
  rotateFilter->SetInput(gradient);
  rotateFilter->GetOutput()->SetRequestedRegion(region);
  rotateFilter->Update();

  //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(rotateFilter->GetOutput(),
  //                   "Debug/ComputeMaskedIsophotes.Isophotes.mha", this->DebugImages);
  //Helpers::Write2DVectorImage(rotateFilter->GetOutput(), "Debug/ComputeMaskedIsophotes.Isophotes.mha");

  std::cout << "ComputeMaskedIsophotesInRegion: CopyRegion" << std::endl;
  ITKHelpers::CopyRegion(rotateFilter->GetOutput(), outputIsophotes, region, region);
  std::cout << "Finish ComputeMaskedIsophotesInRegion" << std::endl;
}

#endif
