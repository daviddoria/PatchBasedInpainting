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

namespace Isophotes
{

template <typename TVectorImageType, typename TIsophoteImageType>
void ComputeColorIsophotesInRegion(const TVectorImageType* const image, const Mask* const mask,
                                   const itk::ImageRegion<2>& region , TIsophoteImageType* const isophotes)
{
  /*
   * 'isophotes' must already be initialized to the right size and allocated.
   */
  
  //EnterFunction("ComputeIsophotes()");
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  ITKHelpers::VectorImageToRGBImage(image, rgbImage);

  //HelpersOutput::WriteImageConditional<RGBImageType>(rgbImage, "Debug/Initialize.rgb.mha", this->DebugImages);

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  FloatScalarImageType::Pointer luminanceImage = FloatScalarImageType::New();
  ITKHelpers::DeepCopy(luminanceFilter->GetOutput(), luminanceImage.GetPointer());

  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel. From TestIsophotes.cpp, it actually seems like not blurring,
  // but using a masked sobel operator produces the most reliable isophotes.
  unsigned int kernelRadius = 0;
  MaskOperations::MaskedBlur(luminanceFilter->GetOutput(), mask,
                             kernelRadius, blurredLuminance.GetPointer());

  //HelpersOutput::WriteImageConditional(blurredLuminance,
  //                                  "Debug/Initialize.blurredLuminance.mha", true);

  //ITKHelpers::InitializeImage(isophotes, image->GetLargestPossibleRegion());
  Isophotes::ComputeMaskedIsophotesInRegion(blurredLuminance.GetPointer(), mask, region, isophotes);

//   if(this->DebugImages)
//     {
//     HelpersOutput::Write2DVectorImage(this->IsophoteImage, "Debug/Initialize.IsophoteImage.mha");
//     }
  //LeaveFunction("ComputeIsophotes()");
}

template <typename TVectorImageType, typename TIsophoteImageType>
void ComputeMaskedIsophotesInRegion(const TVectorImageType* const image, const Mask* const mask,
                                    const itk::ImageRegion<2>& region, TIsophoteImageType* const outputIsophotes)
{
  //Helpers::WriteImageConditional<FloatScalarImageType>(image, "Debug/ComputeMaskedIsophotes.luminance.mha",
//                                                      this->DebugImages);

  typename TIsophoteImageType::Pointer gradient = TIsophoteImageType::New();
  ITKHelpers::InitializeImage(gradient.GetPointer(), image->GetLargestPossibleRegion());
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
  rotateFilter->Update();

  //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(rotateFilter->GetOutput(),
  //                   "Debug/ComputeMaskedIsophotes.Isophotes.mha", this->DebugImages);
  //Helpers::Write2DVectorImage(rotateFilter->GetOutput(), "Debug/ComputeMaskedIsophotes.Isophotes.mha");

  ITKHelpers::CopyRegion(rotateFilter->GetOutput(), outputIsophotes, region, region);
}

} // end namespace

#endif
