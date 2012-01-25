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

#include "Derivatives.h"
#include "Types.h"
#include "Mask.h"
#include "Testing/Testing.h"

static void CreateMask(Mask* const mask);
static void CreateImage(UnsignedCharScalarImageType* const image);

static void TestMaskedDerivative();
static void TestMaskedDerivativePrewitt();
static void TestMaskedDerivativeSobel();
static void TestMaskedDerivativeGaussian();
static void TestMaskedGradientInRegion();
static void TestMaskedDerivativeGaussianInRegion();
static void TestGradientFromDerivatives();
static void TestGradientFromDerivativesInRegion();

int main(int argc, char*argv[])
{
  TestMaskedDerivative();
  TestMaskedDerivativePrewitt();
  TestMaskedDerivativeSobel();
  TestMaskedDerivativeGaussian();
  TestMaskedGradientInRegion();
  TestMaskedDerivativeGaussianInRegion();
  TestGradientFromDerivatives();
  TestGradientFromDerivativesInRegion();

  return EXIT_SUCCESS;
}


void TestMaskedDerivativePrewitt()
{
  //(const TImage* const image, const Mask* const mask, const unsigned int direction, FloatScalarImageType* const output);
  throw;
}


void TestMaskedDerivativeSobel()
{
  //(const TImage* const image, const Mask* const mask, const unsigned int direction, FloatScalarImageType* const output);
  throw;
}

void TestMaskedDerivativeGaussian()
{
//const TImage* const image, const Mask* const mask, const unsigned int direction, FloatScalarImageType* const output);
throw;
}


void TestMaskedDerivativeGaussianInRegion()
{
//const TImage* const image, const Mask* const mask, const unsigned int direction,
  //                                    const itk::ImageRegion<2>& region, FloatScalarImageType* const output);
throw;
}

void TestMaskedGradientInRegion()
{
  //const TImage* const image, const Mask* const mask, const itk::ImageRegion<2>& region, FloatVector2ImageType* const output);
  throw;
}

void TestGradientFromDerivatives()
{
//const itk::Image<TPixel, 2>* const xDerivative,
 //                            const typename itk::Image<TPixel, 2>* const yDerivative, itk::Image<itk::CovariantVector<TPixel, 2> >* const output);
 throw;
}


void TestGradientFromDerivativesInRegion()
{
  //const itk::Image<TPixel, 2>* const xDerivative, const itk::Image<TPixel, 2>* const yDerivative,
    //                                 const itk::ImageRegion<2>& region, itk::Image<itk::CovariantVector<TPixel, 2> >* const output);
  throw;
}

void TestMaskedDerivative()
{
  Mask::Pointer mask = Mask::New();
  CreateMask(mask);

  UnsignedCharScalarImageType::Pointer image = UnsignedCharScalarImageType::New();
  CreateImage(image);

  unsigned int direction = 0; // X-direction
  FloatScalarImageType::Pointer output = FloatScalarImageType::New();
  Derivatives::MaskedDerivative<UnsignedCharScalarImageType>(image, mask, direction, output);

  FloatScalarImageType::Pointer correctDerivative = FloatScalarImageType::New();

  if(!Testing::ImagesEqual<FloatScalarImageType>(output, correctDerivative))
    {
    throw std::runtime_error("MaskedDerivative output incorrect!");
    }
}

void CreateMask(Mask* const mask)
{
  itk::Index<2> maskCorner;
  maskCorner.Fill(0);

  itk::Size<2> maskSize;
  maskSize.Fill(100);

  itk::ImageRegion<2> maskRegion(maskCorner, maskSize);

  mask->SetRegions(maskRegion);
  mask->Allocate();

  unsigned int indeterminateValue = (static_cast<unsigned int>(mask->GetHoleValue()) + static_cast<unsigned int>(mask->GetValidValue()))/2;

  // Make the left half of the mask the hole, the center indeterminate, and the right half valid.
  for(unsigned int column = 0; column < maskSize[0]; ++column)
    {
    for(unsigned int row = 0; row < maskSize[1]; ++row)
      {
      itk::Index<2> currentIndex;
      currentIndex[0] = column;
      currentIndex[1] = row;
      if(column < maskSize[0] / 3)
        {
        mask->SetPixel(currentIndex, mask->GetHoleValue());
        }
      else if(column < 2*maskSize[0]/3)
        {
        mask->SetPixel(currentIndex, indeterminateValue);
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
  itk::Index<2> corner;
  corner.Fill(0);

  itk::Size<2> size;
  size.Fill(100);

  itk::ImageRegion<2> region(corner, size);

  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(0);

  // Make a white square on a black background
  for(unsigned int column = 25; column < 75; ++column)
    {
    for(unsigned int row = 25; row < 75; ++row)
      {
      itk::Index<2> currentIndex;
      currentIndex[0] = column;
      currentIndex[1] = row;
      image->SetPixel(currentIndex, 255);
      }
    }
}
