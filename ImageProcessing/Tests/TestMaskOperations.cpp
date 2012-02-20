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

// static void TestFindPixelAcrossHole();
// static void TestVectorMaskedBlur();
// static void TestCopySelfPatchIntoHoleOfTargetRegion();
// static void TestCopySourcePatchIntoHoleOfTargetRegion();
// static void TestMaskedBlur();
// static void TestCreatePatchImage();
// static void TestFindHighestValueInMaskedRegion();
// static void TestFindHighestValueInNonZero();
static void TestAverageNonMaskedNeighborValue_Scalar();
static void TestAverageMaskedNeighborValue_Scalar();
static void TestAverageNonMaskedNeighborValue_Vector();
static void TestAverageMaskedNeighborValue_Vector();

int main()
{
  TestAverageNonMaskedNeighborValue_Scalar();
  TestAverageMaskedNeighborValue_Scalar();
  TestAverageNonMaskedNeighborValue_Vector();
  TestAverageMaskedNeighborValue_Vector();
  /*
  TestFindPixelAcrossHole();
  TestVectorMaskedBlur();
  TestCopySelfPatchIntoHoleOfTargetRegion();
  TestCopySourcePatchIntoHoleOfTargetRegion();
  TestMaskedBlur();
  TestCreatePatchImage();
  TestFindHighestValueInMaskedRegion();
  TestFindHighestValueInNonZero();*/


  return EXIT_SUCCESS;
}

// Look from a pixel across the hole in a specified direction and return the pixel that exists on the other side of the hole.
void TestFindPixelAcrossHole()
{
  //itk::Index<2> FindPixelAcrossHole(const itk::Index<2>& queryPixel, const FloatVector2Type& direction, const Mask* const mask);
  throw std::runtime_error("TestFindPixelAcrossHole not yet written!");
}

// Apply the MaskedBlur function to every channel of a VectorImage separately.
void TestVectorMaskedBlur()
{
  //void VectorMaskedBlur(const FloatVectorImageType* inputImage, const Mask* mask, const float blurVariance, FloatVectorImageType* output);
  throw std::runtime_error("TestVectorMaskedBlur not yet written!");
}

void TestCopySelfPatchIntoHoleOfTargetRegion()
{
  //template <class TImage>
  //void CopySelfPatchIntoHoleOfTargetRegion(TImage* image, const Mask* mask,
    //                               const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);
  throw std::runtime_error("TestCopySelfPatchIntoHoleOfTargetRegion not yet written!");
}

void TestCopySourcePatchIntoHoleOfTargetRegion()
{
  //template <class TImage>
  //void CopySourcePatchIntoHoleOfTargetRegion(const TImage* sourceImage, TImage* targetImage, const Mask* mask,
    //                                       const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);
  throw std::runtime_error("TestCopySourcePatchIntoHoleOfTargetRegion not yet written!");
}

void TestMaskedBlur()
{
  //template <typename TImage>
  //void MaskedBlur(const TImage* inputImage, const Mask* mask, const float blurVariance, TImage* output);
  throw std::runtime_error("TestMaskedBlur not yet written!");
}

void TestCreatePatchImage()
{
  //template<typename TImage>
  //void CreatePatchImage(TImage* image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, const Mask* mask, TImage* result);
  throw std::runtime_error("TestCreatePatchImage not yet written!");
}

void TestFindHighestValueInMaskedRegion()
{
  // Return the highest value of the specified image out of the pixels under a specified BoundaryImage.
  //template<typename TImage>
  //itk::Index<2> FindHighestValueInMaskedRegion(const TImage* const image, float& maxValue, const Mask* const maskImage);
  throw std::runtime_error("TestFindHighestValueInMaskedRegion not yet written!");
}

void TestFindHighestValueInNonZero()
{
  //template<typename TImage, typename TRegionIndicatorImage>
  //itk::Index<2> FindHighestValueInNonZero(const TImage* const image, float& maxValue, const TRegionIndicatorImage* const maskImage);
  throw std::runtime_error("TestFindHighestValueInNonZero not yet written!");
}

void TestAverageNonMaskedNeighborValue_Scalar()
{
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{10,10}};
  itk::ImageRegion<2> imageRegion(imageCorner, imageSize);

  typedef itk::Image<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(imageRegion);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(imageRegion);
  mask->Allocate();

  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  float maskedValue = 100;
  float unmaskedValue = 75;
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < 5)
      {
      imageIterator.Set(maskedValue);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetHoleValue());
      }
    else
      {
      imageIterator.Set(unmaskedValue);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetValidValue());
      }
    ++imageIterator;
    }

  itk::Index<2> queryPixel = {{5,5}};
  ImageType::PixelType averageNonMaskedNeighborValue =
    MaskOperations::AverageNonMaskedNeighborValue(image.GetPointer(), mask, queryPixel);

  if(averageNonMaskedNeighborValue != unmaskedValue)
  {
    std::stringstream ss;
    ss << "AverageNonMaskedNeighborValue_Scalar was " << averageNonMaskedNeighborValue << " but should be " << unmaskedValue;
    throw std::runtime_error(ss.str());
  }
}

void TestAverageMaskedNeighborValue_Scalar()
{
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{10,10}};
  itk::ImageRegion<2> imageRegion(imageCorner, imageSize);

  typedef itk::Image<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(imageRegion);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(imageRegion);
  mask->Allocate();

  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  float maskedValue = 100;
  float unmaskedValue = 75;
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < 5)
      {
      imageIterator.Set(maskedValue);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetHoleValue());
      }
    else
      {
      imageIterator.Set(unmaskedValue);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetValidValue());
      }
    ++imageIterator;
    }

  itk::Index<2> queryPixel = {{5,5}};
  ImageType::PixelType averageMaskedNeighborValue =
    MaskOperations::AverageMaskedNeighborValue(image.GetPointer(), mask, queryPixel);

  if(averageMaskedNeighborValue != maskedValue)
  {
    std::stringstream ss;
    ss << "AverageMaskedNeighborValue_Scalar was " << averageMaskedNeighborValue << " but should be " << maskedValue;
    throw std::runtime_error(ss.str());
  }

}


void TestAverageNonMaskedNeighborValue_Vector()
{
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{10,10}};
  itk::ImageRegion<2> imageRegion(imageCorner, imageSize);

  const unsigned int imagePixelDimension = 3;
  typedef itk::VectorImage<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetNumberOfComponentsPerPixel(imagePixelDimension);
  image->SetRegions(imageRegion);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(imageRegion);
  mask->Allocate();

  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  ImageType::PixelType maskedPixel;
  maskedPixel.SetSize(imagePixelDimension);
  maskedPixel.Fill(100);

  ImageType::PixelType unmaskedPixel;
  unmaskedPixel.SetSize(imagePixelDimension);
  unmaskedPixel.Fill(75);

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < 5)
      {
      imageIterator.Set(maskedPixel);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetHoleValue());
      }
    else
      {
      imageIterator.Set(unmaskedPixel);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetValidValue());
      }
    ++imageIterator;
    }

  itk::Index<2> queryPixel = {{5,5}};
  ImageType::PixelType averageNonMaskedNeighborValue =
    MaskOperations::AverageNonMaskedNeighborValue(image.GetPointer(), mask, queryPixel);

  if(averageNonMaskedNeighborValue != unmaskedPixel)
  {
    std::stringstream ss;
    ss << "AverageNonMaskedNeighborValue_Vector was " << averageNonMaskedNeighborValue << " but should be " << unmaskedPixel;
    throw std::runtime_error(ss.str());
  }
}

void TestAverageMaskedNeighborValue_Vector()
{
  itk::Index<2> imageCorner = {{0,0}};
  itk::Size<2> imageSize = {{10,10}};
  itk::ImageRegion<2> imageRegion(imageCorner, imageSize);

  const unsigned int imagePixelDimension = 3;
  typedef itk::VectorImage<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetNumberOfComponentsPerPixel(imagePixelDimension);
  image->SetRegions(imageRegion);
  image->Allocate();

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(imageRegion);
  mask->Allocate();

  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  ImageType::PixelType maskedPixel;
  maskedPixel.SetSize(imagePixelDimension);
  maskedPixel.Fill(100);

  ImageType::PixelType unmaskedPixel;
  unmaskedPixel.SetSize(imagePixelDimension);
  unmaskedPixel.Fill(75);

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < 5)
      {
      imageIterator.Set(maskedPixel);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetHoleValue());
      }
    else
      {
      imageIterator.Set(unmaskedPixel);
      mask->SetPixel(imageIterator.GetIndex(), mask->GetValidValue());
      }
    ++imageIterator;
    }

  itk::Index<2> queryPixel = {{5,5}};
  ImageType::PixelType averageMaskedNeighborValue =
    MaskOperations::AverageMaskedNeighborValue(image.GetPointer(), mask, queryPixel);

  if(averageMaskedNeighborValue != maskedPixel)
  {
    std::stringstream ss;
    ss << "AverageMaskedNeighborValue_Vector was " << averageMaskedNeighborValue << " but should be " << maskedPixel;
    throw std::runtime_error(ss.str());
  }
}
