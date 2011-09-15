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
#include "SelfPatchMatch.h"

bool TestSamePatch(ColorImageType::Pointer image, UnsignedCharImageType::Pointer mask);
bool TestDifferentPatch(ColorImageType::Pointer image, UnsignedCharImageType::Pointer mask);
bool TestOutsideImage(ColorImageType::Pointer image, UnsignedCharImageType::Pointer mask);

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Only gave " << argc << " arguments!" << std::endl;
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  ColorImageReaderType::Pointer imageReader = ColorImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  UnsignedCharImageReaderType::Pointer maskReader = UnsignedCharImageReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  bool result1 = TestSamePatch(imageReader->GetOutput(), maskReader->GetOutput());
  bool result2 = TestDifferentPatch(imageReader->GetOutput(), maskReader->GetOutput());
  bool result3 = TestOutsideImage(imageReader->GetOutput(), maskReader->GetOutput());
  
  return EXIT_SUCCESS;
}

bool TestSamePatch(ColorImageType::Pointer image, UnsignedCharImageType::Pointer mask)
{
  itk::Index<2> queryPixel;
  queryPixel[0] = 10;
  queryPixel[1] = 10;

  itk::Index<2> fixedPixel;
  fixedPixel[0] = 10;
  fixedPixel[1] = 10;

  unsigned int patchRadius = 5;

  float difference = PatchDifference(image.GetPointer(), mask.GetPointer(), queryPixel, fixedPixel, patchRadius);
  std::cerr << "Difference: " << difference << std::endl;

  if(difference != 0)
    {
    std::cerr << "Error: the difference between the same pixel should be zero!" << std::endl;
    return false;
    }
  return true;
}

bool TestDifferentPatch(ColorImageType::Pointer image, UnsignedCharImageType::Pointer mask)
{
  itk::Index<2> queryPixel;
  queryPixel[0] = 11;
  queryPixel[1] = 10;

  itk::Index<2> fixedPixel;
  fixedPixel[0] = 10;
  fixedPixel[1] = 10;

  unsigned int patchRadius = 5;

  float difference = PatchDifference(image.GetPointer(), mask.GetPointer(), queryPixel, fixedPixel, patchRadius);
  std::cerr << "Difference: " << difference << std::endl;

  if(difference == 0)
    {
    std::cerr << "Error: the difference between non-exact patches should not be zero!" << std::endl;
    return false;
    }

  return true;
}

bool TestOutsideImage(ColorImageType::Pointer image, UnsignedCharImageType::Pointer mask)
{
  itk::Index<2> queryPixel;
  queryPixel[0] = 10;
  queryPixel[1] = 10;

  itk::Index<2> fixedPixel;
  fixedPixel[0] = 0;
  fixedPixel[1] = 0;

  unsigned int patchRadius = 5;

  float difference = PatchDifference(image.GetPointer(), mask.GetPointer(), queryPixel, fixedPixel, patchRadius);
  std::cerr << "Difference: " << difference << std::endl;

  if(difference == 0)
    {
    std::cerr << "Error: the difference between non-exact patches should not be zero!" << std::endl;
    return false;
    }

  return true;
}

