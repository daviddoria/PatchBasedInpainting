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
#include <Mask/Mask.h>

// ITK
#include "itkImageFileReader.h"

/** This function ensures that the difference between a patch and itself is zero.*/
template <typename TImage>
bool TestSamePatch(const TImage* const image, const Mask* const mask);

template <typename TImage>
bool TestDifferentPatch(const TImage* const image, const Mask* const mask);

template <typename TImage>
bool TestOutsideImage(const TImage* const image, const Mask* const mask);

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

  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  bool resultSamePatch = TestSamePatch(imageReader->GetOutput(), maskReader->GetOutput());
  bool resultDifferentPatch = TestDifferentPatch(imageReader->GetOutput(), maskReader->GetOutput());
  bool resultOutsideImage = TestOutsideImage(imageReader->GetOutput(), maskReader->GetOutput());

  if(!resultSamePatch || !resultDifferentPatch || !resultOutsideImage)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

template <typename TImage>
bool TestSamePatch(const TImage* const image, const Mask* const mask)
{
  itk::Index<2> queryPixel = {{10,10}};

  itk::Index<2> fixedPixel = {{10,10}};

  unsigned int patchRadius = 5;

  float difference = PatchDifference(image, mask, queryPixel, fixedPixel, patchRadius);
  std::cerr << "Difference: " << difference << std::endl;

  if(difference != 0)
  {
    std::cerr << "Error: the difference between the same pixel should be zero!" << std::endl;
    return false;
  }
  return true;
}

template <typename TImage>
bool TestDifferentPatch(const TImage* const image, const Mask* const mask)
{
  itk::Index<2> queryPixel = {{11,10}};

  itk::Index<2> fixedPixel = {{10,10}};

  unsigned int patchRadius = 5;

  float difference = PatchDifference(image, mask, queryPixel, fixedPixel, patchRadius);
  std::cerr << "Difference: " << difference << std::endl;

  if(difference == 0)
  {
    std::cerr << "Error: the difference between non-exact patches should not be zero!" << std::endl;
    return false;
  }

  return true;
}

template <typename TImage>
bool TestOutsideImage(const TImage* const image, const Mask* const mask)
{
  itk::Index<2> queryPixel = {{10,10}};

  itk::Index<2> fixedPixel = {{0,0}};

  unsigned int patchRadius = 5;

  float difference = PatchDifference(image, mask, queryPixel, fixedPixel, patchRadius);
  std::cerr << "Difference: " << difference << std::endl;

  if(difference == 0)
  {
    std::cerr << "Error: the difference between non-exact patches should not be zero!" << std::endl;
    return false;
  }

  return true;
}

