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

#include "CriminisiInpainting.h"

// ITK
#include "itkImageFileReader.h"

int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 5)
    {
    std::cerr << "Required arguments: image imageMask patchRadius output" << std::endl;
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  int patchRadius = 0;
  ssPatchRadius >> patchRadius;

  std::string outputFilename = argv[4];
  
  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch radius: " << patchRadius << std::endl;
  std::cout << "Output: " << outputFilename << std::endl;

  typedef  itk::ImageFileReader< FloatVectorImageType > ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  typedef  itk::ImageFileReader< Mask  > MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  CriminisiInpainting inpainting;
  inpainting.SetPatchRadius(patchRadius);
  inpainting.SetImage(imageReader->GetOutput());
  inpainting.SetMask(maskReader->GetOutput());
  inpainting.SetMaxForwardLookPatches(3);
  inpainting.Initialize();
  inpainting.Inpaint();

  Helpers::WriteImage<FloatVectorImageType>(inpainting.GetCurrentOutputImage(), outputFilename + ".mha");
  Helpers::WriteVectorImageAsRGB(inpainting.GetCurrentOutputImage(), outputFilename);

  return EXIT_SUCCESS;
}
