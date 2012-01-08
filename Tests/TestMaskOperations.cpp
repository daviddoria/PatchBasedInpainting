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

#include "MaskOperations.h"

// STL
#include <iostream>
#include <stdexcept>

static void TestFindPixelAcrossHole();
static void TestVectorMaskedBlur();
static void TestCopySelfPatchIntoHoleOfTargetRegion();
static void TestCopySourcePatchIntoHoleOfTargetRegion();
static void TestMaskedBlur();
static void TestCreatePatchImage();
static void TestFindHighestValueInMaskedRegion();
static void TestFindHighestValueInNonZero();

int main()
{
  TestFindPixelAcrossHole();
  TestVectorMaskedBlur();
  TestCopySelfPatchIntoHoleOfTargetRegion();
  TestCopySourcePatchIntoHoleOfTargetRegion();
  TestMaskedBlur();
  TestCreatePatchImage();
  TestFindHighestValueInMaskedRegion();
  TestFindHighestValueInNonZero();

  return EXIT_SUCCESS;
}

// Look from a pixel across the hole in a specified direction and return the pixel that exists on the other side of the hole.
void TestFindPixelAcrossHole()
{
  //itk::Index<2> FindPixelAcrossHole(const itk::Index<2>& queryPixel, const FloatVector2Type& direction, const Mask* const mask);
  throw;
}

// Apply the MaskedBlur function to every channel of a VectorImage separately.
void TestVectorMaskedBlur()
{
  //void VectorMaskedBlur(const FloatVectorImageType* inputImage, const Mask* mask, const float blurVariance, FloatVectorImageType* output);
  throw;
}

void TestCopySelfPatchIntoHoleOfTargetRegion()
{
  //template <class TImage>
  //void CopySelfPatchIntoHoleOfTargetRegion(TImage* image, const Mask* mask,
    //                               const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);
    throw;
}

void TestCopySourcePatchIntoHoleOfTargetRegion()
{
  //template <class TImage>
  //void CopySourcePatchIntoHoleOfTargetRegion(const TImage* sourceImage, TImage* targetImage, const Mask* mask,
    //                                       const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);
    throw;
}

void TestMaskedBlur()
{
  //template <typename TImage>
  //void MaskedBlur(const TImage* inputImage, const Mask* mask, const float blurVariance, TImage* output);
  throw;
}

void TestCreatePatchImage()
{
  //template<typename TImage>
  //void CreatePatchImage(TImage* image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, const Mask* mask, TImage* result);
  throw;
}

void TestFindHighestValueInMaskedRegion()
{
  // Return the highest value of the specified image out of the pixels under a specified BoundaryImage.
  //template<typename TImage>
  //itk::Index<2> FindHighestValueInMaskedRegion(const TImage* const image, float& maxValue, const Mask* const maskImage);
  throw;
}

void TestFindHighestValueInNonZero()
{
  //template<typename TImage, typename TRegionIndicatorImage>
  //itk::Index<2> FindHighestValueInNonZero(const TImage* const image, float& maxValue, const TRegionIndicatorImage* const maskImage);
  throw;
}

