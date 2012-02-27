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

#include "ImageProcessing/MaskOperations.h"
#include "Mask.h"
#include "Testing/Testing.h"

static void CreateMask(Mask* const mask);
static void CreateImage(UnsignedCharScalarImageType* const image);

int main(int argc, char*argv[])
{
  Mask::Pointer mask = Mask::New();
  CreateMask(mask);

  UnsignedCharScalarImageType::Pointer image = UnsignedCharScalarImageType::New();
  CreateImage(image);

  unsigned int direction = 0; // X-direction
  FloatScalarImageType::Pointer output = FloatScalarImageType::New();
  MaskOperations::MaskedLaplacian<UnsignedCharScalarImageType>(image, mask, direction, output);

  FloatScalarImageType::Pointer correctDerivative = FloatScalarImageType::New();

  if(!Testing::ImagesEqual<FloatScalarImageType>(output, correctDerivative))
    {
    throw std::runtime_error("MaskedDerivative output incorrect!");
    }

  return EXIT_SUCCESS;
}

void CreateMask(Mask* const mask)
{
  itk::Index<2> maskCorner = {{0,0}};

  itk::Size<2> maskSize = {{100, 100}};

  itk::ImageRegion<2> maskRegion(maskCorner, maskSize);

  mask->SetRegions(maskRegion);
  mask->Allocate();

  // Make a valid mask with a hole in the middle
  for(unsigned int column = 0; column < maskSize[0]; ++column)
    {
    for(unsigned int row = 0; row < maskSize[1]; ++row)
      {
      itk::Index<2> currentIndex;
      currentIndex[0] = column;
      currentIndex[1] = row;
      if(column > maskSize[0] / 3 && column < 2 * maskSize[0] / 3 &&
         row > maskSize[1] / 3 && row < 2 * maskSize[1] / 3)
        {
        mask->SetPixel(currentIndex, mask->GetHoleValue());
        }
      else
        {
        mask->SetPixel(currentIndex, mask->GetValidValue());
        }
      }
    }
}

void CreateImage(UnsignedCharScalarImageType* const image)
{
  itk::Index<2> corner = {{0,0}};

  itk::Size<2> size = {{100, 100}};

  itk::ImageRegion<2> region(corner, size);

  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(0);

  // Make a white square on a black background
  for(unsigned int column = 0; column < size[0]; ++column)
    {
    for(unsigned int row = 0; row < size[1]; ++row)
      {
      itk::Index<2> currentIndex;
      currentIndex[0] = column;
      currentIndex[1] = row;
      if(column > size[0] / 3 && column < 2 * size[0] / 3 &&
         row > size[1] / 3 && row < 2 * size[1] / 3)
        {
        image->SetPixel(currentIndex, 0);
        }
      else
        {
        image->SetPixel(currentIndex, 255);
        }
      }
    }
}
