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
#include "ImageProcessing/MaskOperations.h"
#include "ImageProcessing/Mask.h"
#include "Helpers/OutputHelpers.h"

// ITK
#include "itkImageFileReader.h"

int main(int argc, char*argv[])
{
  if(argc != 4)
    {
    std::cerr << "Required arguments: image mask output" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string outputFilename = argv[3];
  
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Output: " << outputFilename << std::endl;

  typedef itk::Image<float, 2> ImageType;
  
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename.c_str());

  typedef itk::Image<float, 2> LaplacianImageType;
  LaplacianImageType::Pointer output = LaplacianImageType::New();
  MaskOperations::MaskedLaplacian(imageReader->GetOutput(), mask.GetPointer(), output.GetPointer());

  OutputHelpers::WriteImage(output.GetPointer(), outputFilename);

  return EXIT_SUCCESS;
}
