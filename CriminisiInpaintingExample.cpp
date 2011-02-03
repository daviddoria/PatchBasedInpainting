/*=========================================================================
 *
 *  Copyright David Doria 2010 daviddoria@gmail.com
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

int main(int argc, char *argv[])
{
  if(argc != 4)
    {
    std::cerr << "Only gave " << argc << " arguments!" << std::endl;
    std::cerr << "Required arguments: image imageMask patchRadius" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  int patchRadius;
  ssPatchRadius >> patchRadius;

  std::cout << "Patch radius: " << patchRadius << std::endl;

  ColorImageReaderType::Pointer imageReader = ColorImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  UnsignedCharImageReaderType::Pointer maskReader = UnsignedCharImageReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  CriminisiInpainting<ColorImageType> Inpainting;
  Inpainting.SetPatchRadius(patchRadius);
  std::vector<float> weights(3);
  weights[0] = .33;
  weights[1] = .33;
  weights[2] = .33;
  Inpainting.SetWeights(weights);
  //Inpainting.SetWriteIntermediateImages(true);
  Inpainting.SetWriteIntermediateImages(false);
  Inpainting.SetImage(imageReader->GetOutput());
  Inpainting.SetInputMask(maskReader->GetOutput());
  Inpainting.Inpaint();

  return EXIT_SUCCESS;
}
