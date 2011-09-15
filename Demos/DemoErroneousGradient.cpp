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

#include "itkGradientImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkBinaryDilateImageFilter.h"

void CreateErroneousGradient(std::string imageFilename);
void MaskNewGradientWithOriginalMask(std::string imageFilename, std::string maskFilename);

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

void CreateErroneousGradient(std::string imageFilename)
{
  std::cout << "Reading image: " << imageFilename << std::endl;

  ColorImageReaderType::Pointer imageReader = ColorImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  // Convert the color input image to a grayscale image
  UnsignedCharImageType::Pointer grayscaleImage = UnsignedCharImageType::New();
  ColorToGrayscale<ColorImageType>(imageReader->GetOutput(), grayscaleImage);

  WriteImage<UnsignedCharImageType>(grayscaleImage, "greyscale.png");

  // Blur the image to compute better gradient estimates
  typedef itk::DiscreteGaussianImageFilter<
          UnsignedCharImageType, FloatImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
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
  typedef itk::GradientMagnitudeImageFilter<
      FloatImageType, UnsignedCharImageType>  GradientMagnitudeFilterType;
  GradientMagnitudeFilterType::Pointer gradientMagnitudeFilter = GradientMagnitudeFilterType::New();
  gradientMagnitudeFilter->SetInput(gaussianFilter->GetOutput());
  gradientMagnitudeFilter->Update();

  WriteImage<UnsignedCharImageType>(gradientMagnitudeFilter->GetOutput(), "gradient.png");

}

void MaskNewGradientWithOriginalMask(std::string imageFilename, std::string maskFilename)
{
  // Read image and convert it to grayscale
  ColorImageReaderType::Pointer imageReader = ColorImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  UnsignedCharImageType::Pointer image = UnsignedCharImageType::New();
  ColorToGrayscale<ColorImageType>(imageReader->GetOutput(), image);

  // Read mask image
  UnsignedCharImageReaderType::Pointer maskReader = UnsignedCharImageReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  // Blur the image to compute better gradient estimates
  typedef itk::DiscreteGaussianImageFilter<
          UnsignedCharImageType, FloatImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
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
  typedef itk::GradientMagnitudeImageFilter<
      FloatImageType, UnsignedCharImageType>  GradientMagnitudeFilterType;
  GradientMagnitudeFilterType::Pointer gradientMagnitudeFilter = GradientMagnitudeFilterType::New();
  gradientMagnitudeFilter->SetInput(gaussianFilter->GetOutput());
  gradientMagnitudeFilter->Update();

  WriteImage<UnsignedCharImageType>(gradientMagnitudeFilter->GetOutput(), "gradient.png");


  // Expand the mask - this is necessary to prevent the isophotes from being undefined in the target region
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(5);

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<UnsignedCharImageType, UnsignedCharImageType, StructuringElementType>
          BinaryDilateImageFilterType;

  BinaryDilateImageFilterType::Pointer expandMaskFilter
          = BinaryDilateImageFilterType::New();
  expandMaskFilter->SetInput(maskReader->GetOutput());
  expandMaskFilter->SetKernel(structuringElement);
  expandMaskFilter->Update();

  UnsignedCharImageType::Pointer expandedMask = UnsignedCharImageType::New();
  expandedMask->Graft(expandMaskFilter->GetOutput());

  WriteScaledImage<UnsignedCharImageType>(expandedMask, "ExpandedMasked.png");

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertMaskFilter
          = InvertIntensityImageFilterType::New();
  invertMaskFilter->SetInput(expandedMask);
  invertMaskFilter->Update();

  //WriteScaledImage<UnsignedCharImageType>(invertMaskFilter->GetOutput(), "invertedExpandedMask.mhd");

  // Keep only values outside the masked region
  typedef itk::MaskImageFilter< UnsignedCharImageType, UnsignedCharImageType, UnsignedCharImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(gradientMagnitudeFilter->GetOutput());
  maskFilter->SetMaskImage(invertMaskFilter->GetOutput());
  maskFilter->Update();

  WriteScaledImage<UnsignedCharImageType>(maskFilter->GetOutput(), "MaskedGradientMagnitude.png");
}