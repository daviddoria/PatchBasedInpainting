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

// ITK
#include "itkRGBToLuminanceImageFilter.h"

namespace Isophotes
{
void ComputeColorIsophotesInRegion(const FloatVectorImageType* image, const Mask* mask,
                                   const itk::ImageRegion<2>& region , FloatVector2ImageType* isophotes)
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
  Derivatives::ComputeMaskedIsophotesInRegion(blurredLuminance, mask, region, isophotes);

//   if(this->DebugImages)
//     {
//     HelpersOutput::Write2DVectorImage(this->IsophoteImage, "Debug/Initialize.IsophoteImage.mha");
//     }
  //LeaveFunction("ComputeIsophotes()");
}
} // end namespace
