/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

// ITK
#include "itkImageFileReader.h"

// Driver
#include "Drivers/ClassicalImageInpaintingDebug.hpp"

// Run with: Data/trashcan.png Data/trashcan.mask 15 filled.png
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 5)
  {
    std::cerr << "Required arguments: image.png imageMask.mask patchHalfWidth output.png" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
    {
      std::cerr << argv[i] << " ";
    }
    return EXIT_FAILURE;
  }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchHalfWidth;
  ssPatchHalfWidth << argv[3];
  unsigned int patchHalfWidth = 0;
  ssPatchHalfWidth >> patchHalfWidth;

  std::string outputFileName = argv[4];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "Output: " << outputFileName << std::endl;

  typedef itk::Image<itk::CovariantVector<int, 3>, 2> OriginalImageType;

  typedef  itk::ImageFileReader<OriginalImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

//  OriginalImageType* originalImage = imageReader->GetOutput();

  OriginalImageType::Pointer originalImage = OriginalImageType::New();
  ITKHelpers::DeepCopy(imageReader->GetOutput(), originalImage.GetPointer());

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  ClassicalImageInpaintingDebug(originalImage, mask, patchHalfWidth);

  // If the output filename is a png file, then use the RGBImage writer so that it is first
  // casted to unsigned char. Otherwise, write the file directly.
  if(Helpers::GetFileExtension(outputFileName) == "png")
  {
    ITKHelpers::WriteRGBImage(originalImage.GetPointer(), outputFileName);
  }
  else
  {
    ITKHelpers::WriteImage(originalImage.GetPointer(), outputFileName);
  }

  return EXIT_SUCCESS;
}
