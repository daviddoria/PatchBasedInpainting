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
#include "Mask.h"
#include "PatchBasedInpainting.h"
#include "Types.h"

// ITK
#include "itkImageFileReader.h"
#include "itkRGBToLuminanceImageFilter.h"
#include "itkMaskImageFilter.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

// Qt
#include <QColor>

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  std::cout << "Read image " << imageReader->GetOutput()->GetLargestPossibleRegion() << std::endl;

  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  std::cout << "Read mask " << maskReader->GetOutput()->GetLargestPossibleRegion() << std::endl;

  // Prepare image
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  Helpers::VectorImageToRGBImage(imageReader->GetOutput(), rgbImage);
  maskReader->GetOutput()->ApplyToImage<RGBImageType, QColor>(rgbImage, Qt::black);
  HelpersOutput::WriteImage<RGBImageType>(rgbImage, "Test/TestIsophotes.rgb.mha");

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  HelpersOutput::WriteImage<FloatScalarImageType>(luminanceFilter->GetOutput(), "Test/Luminance.mha");

  PatchBasedInpainting inpainting;
  inpainting.SetDebugImages(true);
  inpainting.SetMask(maskReader->GetOutput());
  inpainting.SetImage(imageReader->GetOutput());
  //Helpers::Write2DVectorImage(inpainting.GetIsophoteImage(), "Test/TestIsophotes.isophotes.mha");
  //inpainting.FindBoundary();

  // After blurVariance == 4, you cannot tell the difference in the output.
  for(unsigned int blurVariance = 0; blurVariance < 5; ++blurVariance)
    {
    std::string fileNumber = Helpers::ZeroPad(blurVariance, 2);

    FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();

    // Blur with a Gaussian kernel
    Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), maskReader->GetOutput(), blurVariance, blurredLuminance);
    std::stringstream ssBlurredLuminance;
    ssBlurredLuminance << "Test/BlurredLuminance_" << fileNumber << ".mha";
    HelpersOutput::WriteImage<FloatScalarImageType>(blurredLuminance, ssBlurredLuminance.str());

    //Helpers::WriteImage<FloatScalarImageType>(blurredLuminance, "Test/TestIsophotes.blurred.mha");
    //inpainting.ComputeMaskedIsophotes(blurredLuminance, maskReader->GetOutput());

    // Boundary isophotes
    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    //maskFilter->SetInput(inpainting.GetIsophoteImage());
    //maskFilter->SetMaskImage(inpainting.GetBoundaryImage());
    maskFilter->Update();

    vtkSmartPointer<vtkPolyData> boundaryIsophotes = vtkSmartPointer<vtkPolyData>::New();
    Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), boundaryIsophotes);
    std::stringstream ssPolyData;
    ssPolyData << "Test/BoundaryIsophotes_" << fileNumber << ".vtp";
    HelpersOutput::WritePolyData(boundaryIsophotes, ssPolyData.str());
    }

  return EXIT_SUCCESS;
}
