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
#include "ImageProcessing/Mask.h"
#include "Helpers/ITKHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageFileWriter.h"

int main(int argc, char *argv[])
{
  itk::Index<2> corner = {{0,0}};
  itk::Size<2> size = {{101,101}};
  itk::ImageRegion<2> region(corner, size);

  typedef itk::Image<unsigned char, 2> ImageType;
  ImageType::Pointer noisyImage = ImageType::New();
  noisyImage->SetRegions(region);
  noisyImage->Allocate();

  ImageType::Pointer averageImage = ImageType::New();
  averageImage->SetRegions(region);
  averageImage->Allocate();

  ImageType::Pointer noisyImageShifted = ImageType::New();
  noisyImageShifted->SetRegions(region);
  noisyImageShifted->Allocate();

  // Create noisy image
  itk::ImageRegionIterator<ImageType> noisyImageIterator(noisyImage, noisyImage->GetLargestPossibleRegion());

  while(!noisyImageIterator.IsAtEnd())
    {
    noisyImageIterator.Set(rand() % 255);
    ++noisyImageIterator;
    }

  // Shift the noisy image
  itk::ImageRegionIteratorWithIndex<ImageType> shiftedImageIterator(noisyImage, noisyImage->GetLargestPossibleRegion());

  while(!shiftedImageIterator.IsAtEnd())
    {
    if(shiftedImageIterator.GetIndex()[0] == 0 || shiftedImageIterator.GetIndex()[1] == 0)
      {
      shiftedImageIterator.Set(noisyImage->GetPixel(shiftedImageIterator.GetIndex()));
      }
    else
      {
      itk::Index<2> index = shiftedImageIterator.GetIndex();
      index[0] -= 1;
      index[1] -= 1;
      shiftedImageIterator.Set(noisyImage->GetPixel(index));
      }
    ++shiftedImageIterator;
    }

  ImageType::PixelType averagePixel = ITKHelpers::AverageInRegion(noisyImage.GetPointer(),
                                                                  noisyImage->GetLargestPossibleRegion());
  ITKHelpers::SetRegionToConstant(averageImage.GetPointer(), averageImage->GetLargestPossibleRegion(), averagePixel);
  
  return EXIT_SUCCESS;
}
