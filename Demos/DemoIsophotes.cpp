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
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>
#include <VTKHelpers/VTKHelpers.h>
#include <Mask/Mask.h>
#include <Mask/MaskOperations.h>

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

  typedef itk::Image<float, 2> FloatVectorImageType;
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
  typedef itk::Image<itk::RGBPixel<unsigned char>, 2> RGBImageType;
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  // TODO: Update this to new API
  //Helpers::VectorImageToRGBImage(imageReader->GetOutput(), rgbImage);
  maskReader->GetOutput()->ApplyToImage(rgbImage.GetPointer(), Qt::black);
  ITKHelpers::WriteImage(rgbImage.GetPointer(), "Test/TestIsophotes.rgb.mha");

  typedef itk::Image<float, 2> FloatScalarImageType;
  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  ITKHelpers::WriteImage(luminanceFilter->GetOutput(), "Test/Luminance.mha");

//   PatchBasedInpainting inpainting;
//   inpainting.SetDebugImages(true);
//   inpainting.SetMask(maskReader->GetOutput());
//   inpainting.SetImage(imageReader->GetOutput());
  //Helpers::Write2DVectorImage(inpainting.GetIsophoteImage(), "Test/TestIsophotes.isophotes.mha");
  //inpainting.FindBoundary();

  // After blurVariance == 4, you cannot tell the difference in the output.
  for(unsigned int blurVariance = 0; blurVariance < 5; ++blurVariance)
  {
    std::string fileNumber = Helpers::ZeroPad(blurVariance, 2);

    FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();

    // Blur with a Gaussian kernel
    MaskOperations::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), maskReader->GetOutput(),
                                                     blurVariance, blurredLuminance);
    std::stringstream ssBlurredLuminance;
    ssBlurredLuminance << "Test/BlurredLuminance_" << fileNumber << ".mha";
    ITKHelpers::WriteImage(blurredLuminance.GetPointer(), ssBlurredLuminance.str());

    //Helpers::WriteImage<FloatScalarImageType>(blurredLuminance, "Test/TestIsophotes.blurred.mha");
    //inpainting.ComputeMaskedIsophotes(blurredLuminance, maskReader->GetOutput());

    // Boundary isophotes
    typedef itk::Image<itk::CovariantVector<float, 2>, 2> FloatVector2ImageType;
    typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    //maskFilter->SetInput(inpainting.GetIsophoteImage());
    //maskFilter->SetMaskImage(inpainting.GetBoundaryImage());
    maskFilter->Update();

    vtkSmartPointer<vtkPolyData> boundaryIsophotes = vtkSmartPointer<vtkPolyData>::New();
    // TODO: Update this to new API
    //Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), boundaryIsophotes);
    std::stringstream ssPolyData;
    ssPolyData << "Test/BoundaryIsophotes_" << fileNumber << ".vtp";
    VTKHelpers::WritePolyData(boundaryIsophotes, ssPolyData.str());
  }

  return EXIT_SUCCESS;
}
