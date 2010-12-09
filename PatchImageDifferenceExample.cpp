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

  typedef itk::RegionOfInterestImageFilter< ColorImageType,
                                            ColorImageType > ExtractImageFilterType;
  ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
  extractImageFilter->SetRegionOfInterest(GetRegionAroundPixel(queryPixel, 3u));
  extractImageFilter->SetInput(imageReader->GetOutput());
  extractImageFilter->Update();

  FloatImageType::Pointer differenceImage = FloatImageType::New();
  PatchImageDifference(imageReader->GetOutput(), maskReader->GetOutput(), extractImageFilter->GetOutput(), differenceImage);

  typedef itk::Image< float, 2 > FloatImageType;
  itk::ImageFileWriter<FloatImageType>::Pointer writer =
    itk::ImageFileWriter<FloatImageType>::New();
  writer->SetFileName(outputFilename);
  writer->SetInput(differenceImage);
  writer->Update();

  return EXIT_SUCCESS;
}
