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

#include "Isophotes.h"

// Custom
#include "Derivatives.h"
#include "Helpers/Helpers.h"
#include "MaskOperations.h"
#include "RotateVectors.h"

// ITK
#include "itkRGBToLuminanceImageFilter.h"

namespace Isophotes
{

void ComputeMaskedIsophotesInRegion(const FloatScalarImageType* const image, const Mask* const mask, const itk::ImageRegion<2>& region,
                                    FloatVector2ImageType* const outputIsophotes)
{
  //Helpers::WriteImageConditional<FloatScalarImageType>(image, "Debug/ComputeMaskedIsophotes.luminance.mha", this->DebugImages);

  FloatVector2ImageType::Pointer gradient = FloatVector2ImageType::New();
  ITKHelpers::InitializeImage<FloatVector2ImageType>(gradient, image->GetLargestPossibleRegion());
  Derivatives::MaskedGradientInRegion<FloatScalarImageType>(image, mask, region, gradient);

  //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(gradient, "Debug/ComputeMaskedIsophotes.gradient.mha", this->DebugImages);
  //Helpers::Write2DVectorImage(gradient, "Debug/ComputeMaskedIsophotes.gradient.mha");

  // Rotate the gradient 90 degrees to obtain isophotes from gradient
  typedef itk::UnaryFunctorImageFilter<FloatVector2ImageType, FloatVector2ImageType,
  RotateVectors<FloatVector2ImageType::PixelType,
                FloatVector2ImageType::PixelType> > FilterType;

  FilterType::Pointer rotateFilter = FilterType::New();
  rotateFilter->SetInput(gradient);
  rotateFilter->Update();

  //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(rotateFilter->GetOutput(), "Debug/ComputeMaskedIsophotes.Isophotes.mha", this->DebugImages);
  //Helpers::Write2DVectorImage(rotateFilter->GetOutput(), "Debug/ComputeMaskedIsophotes.Isophotes.mha");

  ITKHelpers::CopyRegion<FloatVector2ImageType>(rotateFilter->GetOutput(), outputIsophotes, region, region);
}

void ComputeColorIsophotesInRegion(const FloatVectorImageType* const image, const Mask* const mask,
                                   const itk::ImageRegion<2>& region , FloatVector2ImageType* const isophotes)
{
  //EnterFunction("ComputeIsophotes()");
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  ITKHelpers::VectorImageToRGBImage(image, rgbImage);

  //HelpersOutput::WriteImageConditional<RGBImageType>(rgbImage, "Debug/Initialize.rgb.mha", this->DebugImages);

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  FloatScalarImageType::Pointer luminanceImage = FloatScalarImageType::New();
  ITKHelpers::DeepCopy<FloatScalarImageType>(luminanceFilter->GetOutput(), luminanceImage);

  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel. From TestIsophotes.cpp, it actually seems like not blurring, but using a masked sobel operator produces the most reliable isophotes.
  unsigned int kernelRadius = 0;
  MaskOperations::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), mask, kernelRadius, blurredLuminance);

  //HelpersOutput::WriteImageConditional<FloatScalarImageType>(blurredLuminance, "Debug/Initialize.blurredLuminance.mha", true);

  ITKHelpers::InitializeImage<FloatVector2ImageType>(isophotes, image->GetLargestPossibleRegion());
  Isophotes::ComputeMaskedIsophotesInRegion(blurredLuminance, mask, region, isophotes);

//   if(this->DebugImages)
//     {
//     HelpersOutput::Write2DVectorImage(this->IsophoteImage, "Debug/Initialize.IsophoteImage.mha");
//     }
  //LeaveFunction("ComputeIsophotes()");
}
} // end namespace
