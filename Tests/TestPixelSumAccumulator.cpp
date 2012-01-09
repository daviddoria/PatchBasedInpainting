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

#include "Patch.h"
#include "PixelSumAccumulator.h"
#include "Types.h"
#include "Testing.h"
#include "ITKHelpers.h"
#include "Mask.h"

static void TestScalar();
static void TestVector();

int main(int argc, char*argv[])
{
  TestScalar();
  TestVector();
  

  return EXIT_SUCCESS;
}

void TestScalar()
{
  const unsigned int patchRadius = 5;
  itk::Index<2> corner;
  corner.Fill(0);
  itk::Size<2> size;
  size.Fill(patchRadius * 2 + 1);
  itk::ImageRegion<2> region(corner,size);
  Patch patch(region);

  FloatScalarImageType::Pointer image = FloatScalarImageType::New();
  Testing::GetBlankImage(image.GetPointer());
  PixelSumAccumulator<FloatScalarImageType::PixelType> pixelSumAccumulator;
  patch.VisitAllPixels<FloatScalarImageType>(image, pixelSumAccumulator);

  // Test blank image sum
  {
  FloatScalarImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatScalarImageType::PixelType expectedSum = 0;
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Blank image sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }
  }

  // Test constant image sum
  {
  const unsigned int constantValue = 6;
  ITKHelpers::SetImageToConstant(image.GetPointer(), constantValue);
  pixelSumAccumulator.Clear();
  patch.VisitAllPixels<FloatScalarImageType>(image, pixelSumAccumulator);
  FloatScalarImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatScalarImageType::PixelType expectedSum = constantValue * size[0] * size[1];
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Constant image sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }
  }

  // Test masked constant image sum
  {
  const unsigned int constantValue = 6;

  itk::Index<2> centerPatchCorner;
  centerPatchCorner.Fill(Testing::TestImageSize/2 - patchRadius);

  itk::ImageRegion<2> centerRegion(centerPatchCorner, size);
  Patch centerPatch(centerRegion);
  std::cout << "centerPatch: " << centerPatch << std::endl;

  Mask::Pointer mask = Mask::New();
  Testing::GetHalfValidMask(mask.GetPointer());

  ITKHelpers::SetImageToConstant(image.GetPointer(), constantValue);
  pixelSumAccumulator.Clear();
  centerPatch.VisitAllValidPixels<FloatScalarImageType>(image, mask, pixelSumAccumulator);
  FloatScalarImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatScalarImageType::PixelType expectedSum = constantValue * size[1] * (size[0] / 2);
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Masked sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }

  // Test that the call compiles without explitly specifying the template parameter.
  centerPatch.VisitAllValidPixels(image.GetPointer(), mask.GetPointer(), pixelSumAccumulator);

  }
}

void TestVector()
{
  const unsigned int dimension = 3;
  const unsigned int patchRadius = 5;
  itk::Index<2> corner;
  corner.Fill(0);
  itk::Size<2> size;
  size.Fill(patchRadius * 2 + 1);
  itk::ImageRegion<2> region(corner,size);
  Patch patch(region);

  FloatVectorImageType::Pointer image = FloatVectorImageType::New();
  Testing::GetBlankImage(image.GetPointer(), dimension);
  PixelSumAccumulator<FloatVectorImageType::PixelType> pixelSumAccumulator;
  patch.VisitAllPixels<FloatVectorImageType>(image, pixelSumAccumulator);

  // Test blank image sum
  {
  FloatVectorImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatVectorImageType::PixelType expectedSum(dimension);
  expectedSum.Fill = 0;
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Blank image sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }
  }

  // Test constant image sum
  {
  const unsigned int constantValue = 6;
  ITKHelpers::SetImageToConstant(image.GetPointer(), constantValue);
  pixelSumAccumulator.Clear();
  patch.VisitAllPixels<FloatVectorImageType>(image, pixelSumAccumulator);
  FloatVectorImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatVectorImageType::PixelType expectedSum(dimension) = constantValue * size[0] * size[1];
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Constant image sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }
  }

  // Test masked constant image sum
  {
  const unsigned int constantValue = 6;

  itk::Index<2> centerPatchCorner;
  centerPatchCorner.Fill(Testing::TestImageSize/2 - patchRadius);

  itk::ImageRegion<2> centerRegion(centerPatchCorner, size);
  Patch centerPatch(centerRegion);
  std::cout << "centerPatch: " << centerPatch << std::endl;

  Mask::Pointer mask = Mask::New();
  Testing::GetHalfValidMask(mask.GetPointer());

  ITKHelpers::SetImageToConstant(image.GetPointer(), constantValue);
  pixelSumAccumulator.Clear();
  centerPatch.VisitAllValidPixels<FloatVectorImageType>(image, mask, pixelSumAccumulator);
  FloatVectorImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatVectorImageType::PixelType expectedSum = constantValue * size[1] * (size[0] / 2);
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Masked sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }

  // Test that the call compiles without explitly specifying the template parameter.
  centerPatch.VisitAllValidPixels(image.GetPointer(), mask.GetPointer(), pixelSumAccumulator);

  }
}
