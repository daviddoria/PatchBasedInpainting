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

#include "SelfPatchMatch.h"
#include "Helpers.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

int main(int argc, char*argv[])
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

  typedef itk::Image< itk::CovariantVector<unsigned char, 3>, 2 > ColorImageType;
  typedef  itk::ImageFileReader< ColorImageType  > ColorImageReaderType;

  ColorImageReaderType::Pointer imageReader = ColorImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  typedef itk::Image< unsigned char, 2 > UnsignedCharImageType;
  typedef  itk::ImageFileReader< UnsignedCharImageType  > UnsignedCharImageReaderType;
  UnsignedCharImageReaderType::Pointer maskReader = UnsignedCharImageReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  itk::Index<2> queryPixel;
  queryPixel[0] = 40;
  queryPixel[1] = 50;

  itk::Index<2> bestMatch = SelfPatchMatch(imageReader->GetOutput(), maskReader->GetOutput(), queryPixel, 3u);

  std::cout << "Best match to " << queryPixel << " is " << bestMatch << std::endl;

  return EXIT_SUCCESS;
}
