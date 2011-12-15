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

#include "Derivatives.h"

#include "RotateVectors.h"

namespace Derivatives
{

void ComputeMaskedIsophotesInRegion(const FloatScalarImageType* image, const Mask* mask, const itk::ImageRegion<2>& region, FloatVector2ImageType* outputIsophotes)
{
  try
  {
    //Helpers::WriteImageConditional<FloatScalarImageType>(image, "Debug/ComputeMaskedIsophotes.luminance.mha", this->DebugImages);

    FloatVector2ImageType::Pointer gradient = FloatVector2ImageType::New();
    Helpers::InitializeImage<FloatVector2ImageType>(gradient, image->GetLargestPossibleRegion());
    MaskedGradientInRegion<FloatScalarImageType>(image, mask, region, gradient);

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

    Helpers::CopyPatch<FloatVector2ImageType>(rotateFilter->GetOutput(), outputIsophotes, region, region);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeMaskedIsophotes!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

} // end namespace
