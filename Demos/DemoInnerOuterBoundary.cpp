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

// Custom
#include "Helpers.h"
#include "HelpersOutput.h"
#include "Types.h"

// ITK
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryContourImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkCompose3DCovariantVectorImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkGradientImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

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
  typedef itk::ImageFileReader<UnsignedCharScalarImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(filename);
  reader->Update();

  // Ensure the image is binary
  typedef itk::BinaryThresholdImageFilter <UnsignedCharScalarImageType, UnsignedCharScalarImageType>
          BinaryThresholdImageFilterType;

  BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(reader->GetOutput());
  thresholdFilter->SetLowerThreshold(122);
  thresholdFilter->SetUpperThreshold(255);
  thresholdFilter->SetInsideValue(255);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->Update();

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(thresholdFilter->GetOutput(), "Binary.png");

  // Find the outer boundary
  typedef itk::BinaryContourImageFilter <UnsignedCharScalarImageType, UnsignedCharScalarImageType >
          binaryContourImageFilterType;
  binaryContourImageFilterType::Pointer outerBoundaryFilter
          = binaryContourImageFilterType::New ();
  outerBoundaryFilter->SetInput(thresholdFilter->GetOutput());
  outerBoundaryFilter->SetForegroundValue(0);
  outerBoundaryFilter->SetBackgroundValue(255);
  outerBoundaryFilter->Update();

  // Invert the result
  typedef itk::InvertIntensityImageFilter <UnsignedCharScalarImageType>
          InvertIntensityImageFilterType;
  InvertIntensityImageFilterType::Pointer invertIntensityFilter
          = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(outerBoundaryFilter->GetOutput());
  invertIntensityFilter->Update();

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(invertIntensityFilter->GetOutput(), "OuterBoundary.png");

  // Find the inner boundary
  binaryContourImageFilterType::Pointer innerBoundaryFilter = binaryContourImageFilterType::New ();
  innerBoundaryFilter->SetInput(thresholdFilter->GetOutput());
  innerBoundaryFilter->SetForegroundValue(255);
  innerBoundaryFilter->SetBackgroundValue(0);
  innerBoundaryFilter->Update();

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(innerBoundaryFilter->GetOutput(), "InnerBoundary.png");

  typedef itk::Compose3DCovariantVectorImageFilter<UnsignedCharScalarImageType,
                              UnsignedCharVectorImageType> ComposeCovariantVectorImageFilterType;

  ComposeCovariantVectorImageFilterType::Pointer composeFilter = ComposeCovariantVectorImageFilterType::New();
  composeFilter->SetInput1(thresholdFilter->GetOutput());
  composeFilter->SetInput2(thresholdFilter->GetOutput());
  composeFilter->SetInput3(thresholdFilter->GetOutput());
  composeFilter->Update();

  HelpersOutput::WriteImage<UnsignedCharVectorImageType>(composeFilter->GetOutput(), "Composed.png");

  /*
  typedef itk::ImageDuplicator< ImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(image);
  duplicator->Update();
  ImageType::Pointer clonedImage = duplicator->GetOutput();
  */
  itk::ImageRegionConstIterator<UnsignedCharScalarImageType> outerBoundaryIterator(invertIntensityFilter->GetOutput(),invertIntensityFilter->GetOutput()->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<UnsignedCharScalarImageType> innerBoundaryIterator(innerBoundaryFilter->GetOutput(),innerBoundaryFilter->GetOutput()->GetLargestPossibleRegion());
  itk::ImageRegionIterator<UnsignedCharVectorImageType> outputIterator(composeFilter->GetOutput(),composeFilter->GetOutput()->GetLargestPossibleRegion());

  UnsignedCharVectorImageType::PixelType red;
  red[0] = 255;
  red[1] = 0;
  red[2] = 0;
  UnsignedCharVectorImageType::PixelType green;
  green[0] = 0;
  green[1] = 255;
  green[2] = 0;
  while(!outputIterator.IsAtEnd())
    {
    //std::cout << (int)innerBoundaryIterator.Get() << std::endl;
    if(innerBoundaryIterator.Get() > 0)
      {
      outputIterator.Set(red);
      }

    if(outerBoundaryIterator.Get() > 0)
      {
      outputIterator.Set(green);
      }

    ++innerBoundaryIterator;
    ++outerBoundaryIterator;
    ++outputIterator;
    }

  HelpersOutput::WriteImage<UnsignedCharVectorImageType>(composeFilter->GetOutput(), "BothBoundaries.png");

  return EXIT_SUCCESS;
}
