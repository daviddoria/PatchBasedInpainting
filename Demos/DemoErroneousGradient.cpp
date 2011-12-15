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
#include "Types.h"
#include "Helpers.h"
#include "HelpersOutput.h"

// ITK
#include "itkBinaryDilateImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkGradientImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"

void CreateErroneousGradient(const std::string& imageFilename);
void MaskNewGradientWithOriginalMask(const std::string& imageFilename, const std::string& maskFilename);

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: image imageMask" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  CreateErroneousGradient(imageFilename);

  MaskNewGradientWithOriginalMask(imageFilename, maskFilename);

  return EXIT_SUCCESS;
}

void CreateErroneousGradient(const std::string& imageFilename)
{
  std::cout << "Reading image: " << imageFilename << std::endl;

  typedef itk::ImageFileReader<RGBImageType> ReaderType;
  ReaderType::Pointer imageReader = ReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  // Convert the color input image to a grayscale image
  UnsignedCharScalarImageType::Pointer grayscaleImage = UnsignedCharScalarImageType::New();
  //Helpers::ColorToGrayscale<RGBImageType>(imageReader->GetOutput(), grayscaleImage);

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, UnsignedCharScalarImageType> LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(imageReader->GetOutput());
  luminanceFilter->Update();
  Helpers::DeepCopy<UnsignedCharScalarImageType>(luminanceFilter->GetOutput(), grayscaleImage);

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(grayscaleImage, "greyscale.png");

  // Blur the image to compute better gradient estimates
  typedef itk::DiscreteGaussianImageFilter<UnsignedCharScalarImageType, FloatScalarImageType >  GaussianFilterType;
  GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
  gaussianFilter->SetInput(grayscaleImage);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  //WriteImage<FloatImageType>(gaussianFilter->GetOutput(), "gaussianBlur.mhd"); // this cannot be png because they are floats

  /*
  // Compute the gradient
  typedef itk::GradientImageFilter<
      FloatImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  WriteImage<VectorImageType>(gradientFilter->GetOutput(), "gradient.mhd"); // this cannot be png because they are floats
  */

  // Compute the gradient magnitude
  typedef itk::GradientMagnitudeImageFilter<FloatScalarImageType, UnsignedCharScalarImageType>  GradientMagnitudeFilterType;
  GradientMagnitudeFilterType::Pointer gradientMagnitudeFilter = GradientMagnitudeFilterType::New();
  gradientMagnitudeFilter->SetInput(gaussianFilter->GetOutput());
  gradientMagnitudeFilter->Update();

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(gradientMagnitudeFilter->GetOutput(), "gradient.png");

}

void MaskNewGradientWithOriginalMask(const std::string& imageFilename, const std::string& maskFilename)
{
  // Read image and convert it to grayscale
  typedef itk::ImageFileReader<RGBImageType> RGBReaderType;
  RGBReaderType::Pointer imageReader = RGBReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  UnsignedCharScalarImageType::Pointer image = UnsignedCharScalarImageType::New();
  //Helpers::ColorToGrayscale<RGBImageType>(imageReader->GetOutput(), image);
  Helpers::FilterImage<RGBImageType, UnsignedCharScalarImageType, itk::RGBToLuminanceImageFilter<RGBImageType, UnsignedCharScalarImageType> >(imageReader->GetOutput(), image);

  // Read mask image
  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  // Blur the image to compute better gradient estimates
  typedef itk::DiscreteGaussianImageFilter<UnsignedCharScalarImageType, FloatScalarImageType >  GaussianFilterType;
  GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
  gaussianFilter->SetInput(image);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  //WriteImage<FloatImageType>(gaussianFilter->GetOutput(), "gaussianBlur.mhd"); // this cannot be png because they are floats

  /*
  // Compute the gradient
  typedef itk::GradientImageFilter<
      FloatImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  WriteImage<VectorImageType>(gradientFilter->GetOutput(), "gradient.mhd"); // this cannot be png because they are floats
  */

  // Compute the gradient magnitude
  typedef itk::GradientMagnitudeImageFilter<FloatScalarImageType, UnsignedCharScalarImageType>  GradientMagnitudeFilterType;
  GradientMagnitudeFilterType::Pointer gradientMagnitudeFilter = GradientMagnitudeFilterType::New();
  gradientMagnitudeFilter->SetInput(gaussianFilter->GetOutput());
  gradientMagnitudeFilter->Update();

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(gradientMagnitudeFilter->GetOutput(), "gradient.png");


  // Expand the mask - this is necessary to prevent the isophotes from being undefined in the target region
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(5);

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<UnsignedCharScalarImageType, UnsignedCharScalarImageType, StructuringElementType>
          BinaryDilateImageFilterType;

  BinaryDilateImageFilterType::Pointer expandMaskFilter
          = BinaryDilateImageFilterType::New();
  expandMaskFilter->SetInput(maskReader->GetOutput());
  expandMaskFilter->SetKernel(structuringElement);
  expandMaskFilter->Update();

  UnsignedCharScalarImageType::Pointer expandedMask = UnsignedCharScalarImageType::New();
  expandedMask->Graft(expandMaskFilter->GetOutput());

  HelpersOutput::WriteScaledScalarImage<UnsignedCharScalarImageType>(expandedMask, "ExpandedMasked.png");

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharScalarImageType> InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertMaskFilter = InvertIntensityImageFilterType::New();
  invertMaskFilter->SetInput(expandedMask);
  invertMaskFilter->Update();

  //WriteScaledImage<UnsignedCharImageType>(invertMaskFilter->GetOutput(), "invertedExpandedMask.mhd");

  // Keep only values outside the masked region
  typedef itk::MaskImageFilter< UnsignedCharScalarImageType, UnsignedCharScalarImageType, UnsignedCharScalarImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(gradientMagnitudeFilter->GetOutput());
  maskFilter->SetMaskImage(invertMaskFilter->GetOutput());
  maskFilter->Update();

  HelpersOutput::WriteScaledScalarImage<UnsignedCharScalarImageType>(maskFilter->GetOutput(), "MaskedGradientMagnitude.png");
}
