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

  HelpersOutput::WriteImage<RGBImageType>(rgbImage, "Test/TestIsophotes.rgb.mha");

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel
  unsigned int kernelRadius = 5;
  Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), maskReader->GetOutput(), kernelRadius, blurredLuminance);

  HelpersOutput::WriteImage<FloatScalarImageType>(blurredLuminance, "Test/TestIsophotes.blurred.mha");


  PatchBasedInpainting inpainting(NULL, maskReader->GetOutput());
  //inpainting.SetMask(maskReader->GetOutput());
  //inpainting.SetImage(imageReader->GetOutput());

  //inpainting.FindBoundary();

  for(unsigned int blurVariance = 0; blurVariance < 10; ++blurVariance)
    {
    //inpainting.ComputeBoundaryNormals(blurVariance);

    std::stringstream ss;
    ss << "Test/BoundaryNormals_" << blurVariance << ".mha";
    //HelpersOutput::Write2DVectorImage(inpainting.GetBoundaryNormalsImage(), ss.str());

    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    //maskFilter->SetInput(inpainting.GetBoundaryNormalsImage());
    //maskFilter->SetMaskImage(inpainting.GetBoundaryImage());
    maskFilter->Update();

    vtkSmartPointer<vtkPolyData> boundaryNormals = vtkSmartPointer<vtkPolyData>::New();
    Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), boundaryNormals);
    std::stringstream ssPolyData;
    ssPolyData << "Test/BoundaryNormals_" << blurVariance << ".vtp";
    HelpersOutput::WritePolyData(boundaryNormals, ssPolyData.str());
    }

  return EXIT_SUCCESS;
}
