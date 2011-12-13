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

#include "Types.h"
#include "Helpers.h"
#include "HelpersOutput.h"

#include "itkGradientImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryContourImageFilter.h"

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: imageMask" << std::endl;
    return EXIT_FAILURE;
    }

  std::string maskFilename = argv[1];

  std::cout << "Reading image: " << maskFilename << std::endl;
  typedef itk::ImageFileReader<UnsignedCharScalarImageType> ReaderType;
  ReaderType::Pointer maskReader = ReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

   // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

  typedef itk::DiscreteGaussianImageFilter<UnsignedCharScalarImageType, FloatScalarImageType >  filterType;
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(maskReader->GetOutput());
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharScalarImageType> InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(maskReader->GetOutput());
  invertIntensityFilter->Update();

  // Find the boundary
  typedef itk::BinaryContourImageFilter <UnsignedCharScalarImageType, UnsignedCharScalarImageType > binaryContourImageFilterType;
  binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(invertIntensityFilter->GetOutput());
  binaryContourFilter->Update();

  typedef itk::GradientMagnitudeImageFilter< FloatScalarImageType, UnsignedCharScalarImageType>  GradientMagnitudeFilterType;
  GradientMagnitudeFilterType::Pointer gradientMagnitudeFilter = GradientMagnitudeFilterType::New();
  gradientMagnitudeFilter->SetInput(gaussianFilter->GetOutput());
  gradientMagnitudeFilter->Update();

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(gradientMagnitudeFilter->GetOutput(), "BoundaryGradient.png");

  // Only keep the normals at the boundary
  typedef itk::MaskImageFilter< UnsignedCharScalarImageType, UnsignedCharScalarImageType, UnsignedCharScalarImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(gradientMagnitudeFilter->GetOutput());
  maskFilter->SetMaskImage(binaryContourFilter->GetOutput());
  maskFilter->Update();

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(maskFilter->GetOutput(), "MaskedBoundaryGradient.png");

  return EXIT_SUCCESS;
}
