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

#include "Types.h"
#include "Helpers.h"

#include "itkGradientImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryContourImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

int main(int argc, char *argv[])
{
  if(argc != 2)
    {
    std::cerr << "Required arguments: filename" << std::endl;
    return EXIT_FAILURE;
    }

  std::string filename = argv[1];

  std::cout << "Reading image: " << filename << std::endl;

  // Read the image
  UnsignedCharImageReaderType::Pointer reader = UnsignedCharImageReaderType::New();
  reader->SetFileName(filename.c_str());
  reader->Update();

  // Ensure the image is binary
  typedef itk::BinaryThresholdImageFilter <UnsignedCharImageType, UnsignedCharImageType>
          BinaryThresholdImageFilterType;

  BinaryThresholdImageFilterType::Pointer thresholdFilter
          = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(reader->GetOutput());
  thresholdFilter->SetLowerThreshold(122);
  thresholdFilter->SetUpperThreshold(255);
  thresholdFilter->SetInsideValue(255);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->Update();

  WriteImage<UnsignedCharImageType>(thresholdFilter->GetOutput(), "Binary.png");

  // Find the boundary
  {
  typedef itk::BinaryContourImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          binaryContourImageFilterType;
  binaryContourImageFilterType::Pointer binaryContourFilter
          = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(thresholdFilter->GetOutput());
  // correct
  //binaryContourFilter->SetForegroundValue(0);
  //binaryContourFilter->SetBackgroundValue(255);

  binaryContourFilter->SetForegroundValue(0);
  binaryContourFilter->SetBackgroundValue(0);
  binaryContourFilter->Update();

  // Invert the result
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter
          = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(binaryContourFilter->GetOutput());
  invertIntensityFilter->Update();

  WriteImage<UnsignedCharImageType>(invertIntensityFilter->GetOutput(), "OuterBoundary.png");
  }

  {
  typedef itk::BinaryContourImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          binaryContourImageFilterType;
  binaryContourImageFilterType::Pointer binaryContourFilter
          = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(thresholdFilter->GetOutput());
  // correct
  //binaryContourFilter->SetForegroundValue(255);
  //binaryContourFilter->SetBackgroundValue(0);

  binaryContourFilter->SetForegroundValue(255);
  binaryContourFilter->SetBackgroundValue(255);
  binaryContourFilter->Update();

  WriteImage<UnsignedCharImageType>(binaryContourFilter->GetOutput(), "InnerBoundary.png");
  }

  return EXIT_SUCCESS;
}
