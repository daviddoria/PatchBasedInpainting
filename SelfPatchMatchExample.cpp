/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
