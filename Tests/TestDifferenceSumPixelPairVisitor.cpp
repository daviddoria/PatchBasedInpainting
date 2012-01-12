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

#include "ImagePatch.h"
#include "PatchPair.h"
#include "DifferenceSumPixelPairVisitor.h"
#include "Types.h"
#include "Testing.h"
#include "ITKHelpers.h"
#include "Mask.h"

// TODO: Fix this test

static void TestScalar();
static void TestVector();
static void TestOffsets();

int main(int argc, char*argv[])
{
  TestScalar();
//   TestVector();
//   TestOffsets();

  return EXIT_SUCCESS;
}

void TestScalar()
{
  const unsigned int patchRadius = 5;
  itk::Size<2> size;
  size.Fill(patchRadius * 2 + 1);

  itk::Index<2> sourceCorner;
  sourceCorner.Fill(0);
  itk::ImageRegion<2> sourceRegion(sourceCorner, size);
  Patch sourcePatch(sourceRegion);

  FloatScalarImageType::Pointer image = FloatScalarImageType::New();
  const float leftSideValue = 0;
  const float rightSideValue = 3;
  Testing::GetHalfConstantImage(image.GetPointer(), leftSideValue, rightSideValue);

  DifferenceSumPixelPairVisitor<FloatScalarImageType> pixelPairDifferenceSum;

  // Test sum of differences with patches on the same side of the image
  {
  itk::Index<2> targetCorner;
  targetCorner.Fill(5);
  itk::ImageRegion<2> targetRegion(targetCorner, size);
  Patch targetPatch(targetRegion);

  PatchPair patchPair(&sourcePatch, targetPatch);

  patchPair.VisitAllPixels<FloatScalarImageType>(image, pixelPairDifferenceSum);

  FloatScalarImageType::PixelType differenceSum = pixelPairDifferenceSum.GetSum();
  FloatScalarImageType::PixelType expectedDifferenceSum = 0;
  if(differenceSum != expectedDifferenceSum)
    {
    std::stringstream ss;
    ss << "Scalar: Difference of identical patches should be " << expectedDifferenceSum << " but it is " << differenceSum;
    throw std::runtime_error(ss.str());
    }
  }

  // Test patches on opposite sides of the image
  {
  pixelPairDifferenceSum.Clear();
  
  itk::Index<2> targetCorner;
  targetCorner.Fill(60);
  itk::ImageRegion<2> targetRegion(targetCorner, size);
  Patch targetPatch(targetRegion);

  PatchPair patchPair(&sourcePatch, targetPatch);
  
  patchPair.VisitAllPixels<FloatScalarImageType>(image, pixelPairDifferenceSum);
  FloatScalarImageType::PixelType differenceSum = pixelPairDifferenceSum.GetSum();
  FloatScalarImageType::PixelType expectedDifferenceSum = fabs(rightSideValue - leftSideValue) * size[0] * size[1];
  if(differenceSum != expectedDifferenceSum)
    {
    std::stringstream ss;
    ss << "Scalar: Difference sum of patches from opposite sides of the image should be " << expectedDifferenceSum << " but it is " << differenceSum;
    throw std::runtime_error(ss.str());
    }
  }
/*
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
    ss << "Scalar: Masked sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }

  // Test that the call compiles without explitly specifying the template parameter.
  centerPatch.VisitAllValidPixels(image.GetPointer(), mask.GetPointer(), pixelSumAccumulator);

  }*/
}
/*
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
  FloatVectorImageType::PixelType representativePixel(dimension);
  pixelSumAccumulator.Initialize(representativePixel);
  patch.VisitAllPixels<FloatVectorImageType>(image, pixelSumAccumulator);

  // Test blank image sum
  {
  FloatVectorImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatVectorImageType::PixelType expectedSum(dimension);
  expectedSum.Fill(0);
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Vector: Blank image sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }
  }

  // Test constant image sum
  const unsigned int constantScalar = 6;
  FloatVectorImageType::PixelType constantVector(dimension);
  constantVector.Fill(constantScalar);
  {
  ITKHelpers::SetImageToConstant(image.GetPointer(), constantVector);
  pixelSumAccumulator.Clear();
  patch.VisitAllPixels<FloatVectorImageType>(image, pixelSumAccumulator);
  FloatVectorImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatVectorImageType::PixelType expectedSum(dimension);
  expectedSum.Fill(constantScalar * size[0] * size[1]);
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Vector: Constant image sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }
  }

  // Test masked constant image sum
  {
  itk::Index<2> centerPatchCorner;
  centerPatchCorner.Fill(Testing::TestImageSize/2 - patchRadius);

  itk::ImageRegion<2> centerRegion(centerPatchCorner, size);
  Patch centerPatch(centerRegion);
  std::cout << "centerPatch: " << centerPatch << std::endl;

  Mask::Pointer mask = Mask::New();
  Testing::GetHalfValidMask(mask.GetPointer());

  pixelSumAccumulator.Clear();
  centerPatch.VisitAllValidPixels<FloatVectorImageType>(image, mask, pixelSumAccumulator);
  FloatVectorImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatVectorImageType::PixelType expectedSum(dimension);
  expectedSum.Fill(constantScalar * size[1] * (size[0] / 2));
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Vector: Masked sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }

  // Test that the call compiles without explitly specifying the template parameter.
  centerPatch.VisitAllValidPixels(image.GetPointer(), mask.GetPointer(), pixelSumAccumulator);

  }
}

void TestOffsets()
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
  
  std::vector<itk::Offset<2> > offsets;
  itk::Offset<2> offset1;
  offset1.Fill(0);
  
  itk::Offset<2> offset2;
  offset2.Fill(1);
  
  offsets.push_back(offset1);
  offsets.push_back(offset2);

  const unsigned int constantValue = 6;
  ITKHelpers::SetImageToConstant(image.GetPointer(), constantValue);

  patch.VisitOffsets<FloatScalarImageType>(image, offsets, pixelSumAccumulator);

  FloatScalarImageType::PixelType pixelSum = pixelSumAccumulator.GetSum();
  FloatScalarImageType::PixelType expectedSum = constantValue * offsets.size();
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "TestOffsets: Sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }
}*/
