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

// Submodules
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>
#include <VTKHelpers/VTKHelpers.h>
#include <Mask/Mask.h>

// ITK
#include "itkImageFileReader.h"
#include "itkMaskImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

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

  typedef itk::VectorImage<float, 2> FloatVectorImageType;
  typedef itk::Image<float, 2> FloatScalarImageType;

  typedef itk::Image<itk::RGBPixel<unsigned char>, 2> RGBImageType;

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
  // Helpers::VectorImageToRGBImage(imageReader->GetOutput(), rgbImage);

  ITKHelpers::WriteImage(rgbImage.GetPointer(), "Test/TestIsophotes.rgb.mha");

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel
  // unsigned int kernelRadius = 5;
  // TODO: update this call to the new API
//   Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), maskReader->GetOutput(),
//                                             kernelRadius, blurredLuminance);

  ITKHelpers::WriteImage(blurredLuminance.GetPointer(), "Test/TestIsophotes.blurred.mha");

  // PatchBasedInpainting inpainting(NULL, maskReader->GetOutput());
  //inpainting.SetMask(maskReader->GetOutput());
  //inpainting.SetImage(imageReader->GetOutput());

  //inpainting.FindBoundary();

  for(unsigned int blurVariance = 0; blurVariance < 10; ++blurVariance)
  {
    //inpainting.ComputeBoundaryNormals(blurVariance);

    std::stringstream ss;
    ss << "Test/BoundaryNormals_" << blurVariance << ".mha";
    //HelpersOutput::Write2DVectorImage(inpainting.GetBoundaryNormalsImage(), ss.str());

    typedef itk::Image<itk::CovariantVector<float, 2>, 2> FloatVector2ImageType;
    typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;

    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    //maskFilter->SetInput(inpainting.GetBoundaryNormalsImage());
    //maskFilter->SetMaskImage(inpainting.GetBoundaryImage());
    maskFilter->Update();

    vtkSmartPointer<vtkPolyData> boundaryNormals = vtkSmartPointer<vtkPolyData>::New();
    // TODO: Update this call to the new API
    // Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), boundaryNormals);
    std::stringstream ssPolyData;
    ssPolyData << "Test/BoundaryNormals_" << blurVariance << ".vtp";
    VTKHelpers::WritePolyData(boundaryNormals, ssPolyData.str());
  }

  return EXIT_SUCCESS;
}
