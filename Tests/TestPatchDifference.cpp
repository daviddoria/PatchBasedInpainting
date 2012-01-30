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

#include "PatchDifferencePixelWiseSum.h"
#include "PixelDifference.h"

// Custom
#include "Testing.h"
#include "Types.h"

// ITK
#include "itkImageRegionIterator.h"

// STL
#include <iostream>
#include <stdexcept>

static void FullPatchScalarComparison();
static void FullPatchVectorComparison();

static void PartialPatchScalarComparison();
static void PartialPatchVectorComparison();

int main(int argc, char*argv[])
{
  try
  {
    FullPatchScalarComparison();
    FullPatchVectorComparison();
    PartialPatchScalarComparison();
    PartialPatchVectorComparison();
  }
  catch (std::runtime_error ex)
  {
    std::cerr << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void FullPatchScalarComparison()
{
  std::cout << "FullPatchScalarComparison()" << std::endl;
  FloatScalarImageType::Pointer scalarImage = FloatScalarImageType::New();
  Testing::GetBlankImage<FloatScalarImageType>(scalarImage);

  // Make the left half of the image 0, and the right half 5
  itk::ImageRegionIterator<FloatScalarImageType> imageIterator(scalarImage, scalarImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < static_cast<int>(scalarImage->GetLargestPossibleRegion().GetSize()[0]/2))
      {
      imageIterator.Set(0);
      }
    else
      {
      imageIterator.Set(5);
      }

    ++imageIterator;
    }

  itk::Size<2> patchSize;
  patchSize.Fill(10);

  std::cout << "Full patch different test." << std::endl;
  // Full patch is different
  {
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(0);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> sourcePatch(scalarImage, sourceRegion, true);

  itk::Index<2> targetCorner;
  targetCorner.Fill(scalarImage->GetLargestPossibleRegion().GetSize()[0]/2 + 4); // No magic about 4, just want a patch on the right side of the image
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> targetPatch(scalarImage, targetRegion, true);
  std::cout << "targetPatch: " << targetPatch << std::endl;

  PatchPair<FloatScalarImageType> patchPair(&sourcePatch, targetPatch);
  PatchDifferencePixelWiseSum<FloatScalarImageType, PixelDifference> scalar_patchDifferencePixelWiseSum;
  scalar_patchDifferencePixelWiseSum.SetImage(scalarImage);
  float difference = scalar_patchDifferencePixelWiseSum.Difference(patchPair);

  std::cout << "Number of pixels: " << targetPatch.GetRegion().GetNumberOfPixels() << std::endl;

  float correctDifference = targetPatch.GetRegion().GetNumberOfPixels() * 5;

  if(difference != correctDifference)
    {
    std::stringstream ss;
    ss << "Difference " << difference << " does not match correctDifference " << correctDifference;
    throw std::runtime_error(ss.str());
    }
  }

  std::cout << "Identical patch test." << std::endl;
  // Patches are identical
  {
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(0);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> sourcePatch(scalarImage, sourceRegion, true);

  itk::Index<2> targetCorner;
  targetCorner.Fill(10); // No magic about 10, just want a patch not at (0,0) but still fully on the left side of the image
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> targetPatch(scalarImage, targetRegion, true);

  PatchPair<FloatScalarImageType> patchPair(&sourcePatch, targetPatch);
  PatchDifferencePixelWiseSum<FloatScalarImageType, PixelDifference> scalar_patchDifferencePixelWiseSum;
  scalar_patchDifferencePixelWiseSum.SetImage(scalarImage);
  float difference = scalar_patchDifferencePixelWiseSum.Difference(patchPair);

  float correctDifference = 0;
  if(difference != correctDifference)
    {
    std::stringstream ss;
    ss << "Difference " << difference << " does not match correctDifference " << correctDifference;
    throw std::runtime_error(ss.str());
    }
  }
}

void FullPatchVectorComparison()
{
  std::cout << "FullPatchVectorComparison()" << std::endl;

  const unsigned int dimension = 3;
  FloatVectorImageType::Pointer vectorImage = FloatVectorImageType::New();
  Testing::GetBlankImage<FloatVectorImageType>(vectorImage, dimension);

  // Make the left half of the image (0,0,0) and the right half (5,6,7)
  itk::ImageRegionIterator<FloatVectorImageType> imageIterator(vectorImage, vectorImage->GetLargestPossibleRegion());

  itk::VariableLengthVector<float> leftHalfPixel;
  leftHalfPixel.SetSize(dimension);
  leftHalfPixel.Fill(0);
  
  itk::VariableLengthVector<float> rightHalfPixel;
  rightHalfPixel.SetSize(dimension);
  rightHalfPixel[0] = 5;
  rightHalfPixel[1] = 6;
  rightHalfPixel[2] = 7;
  
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < static_cast<int>(vectorImage->GetLargestPossibleRegion().GetSize()[0]/2))
      {
      imageIterator.Set(leftHalfPixel);
      }
    else
      {
      imageIterator.Set(rightHalfPixel);
      }

    ++imageIterator;
    }

  itk::Size<2> patchSize;
  patchSize.Fill(10);

  // Full patches differ
  std::cout << "Full patch different test." << std::endl;
  {
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(0);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatVectorImageType> sourcePatch(vectorImage, sourceRegion, true);

  itk::Index<2> targetCorner;
  targetCorner.Fill(vectorImage->GetLargestPossibleRegion().GetSize()[0]/2 + 4); // No magic about 4, just want a patch on the right side of the image
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatVectorImageType> targetPatch(vectorImage, targetRegion, true);

  PatchPair<FloatVectorImageType> patchPair(&sourcePatch, targetPatch);
  PatchDifferencePixelWiseSum<FloatVectorImageType, PixelDifference> vector_patchDifferencePixelWiseSum;
  vector_patchDifferencePixelWiseSum.SetImage(vectorImage);
  float difference = vector_patchDifferencePixelWiseSum.Difference(patchPair);

  float correctDifference = targetRegion.GetNumberOfPixels() * 18; // 18 = 5+6+7, the sum of the elements of 'rightHalfPixel'
  if(difference != correctDifference)
    {
    std::stringstream ss;
    ss << "Difference " << difference << " does not match correctDifference " << correctDifference;
    throw std::runtime_error(ss.str());
    }
  }

  // Full patches identical
  std::cout << "Identical patch test." << std::endl;
  {
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(5);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatVectorImageType> sourcePatch(vectorImage, sourceRegion, true);

  itk::Index<2> targetCorner;
  targetCorner.Fill(5);
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatVectorImageType> targetPatch(vectorImage, targetRegion, true);

  PatchPair<FloatVectorImageType> patchPair(&sourcePatch, targetPatch);
  PatchDifferencePixelWiseSum<FloatVectorImageType, PixelDifference> vector_patchDifferencePixelWiseSum;
  vector_patchDifferencePixelWiseSum.SetImage(vectorImage);
  float difference = vector_patchDifferencePixelWiseSum.Difference(patchPair);

  float correctDifference = 0;
  if(difference != correctDifference)
    {
    std::stringstream ss;
    ss << "Difference " << difference << " does not match correctDifference " << correctDifference;
    throw std::runtime_error(ss.str());
    }
  }
}


void PartialPatchScalarComparison()
{
  std::cout << "PartialPatchScalarComparison()" << std::endl;
  FloatScalarImageType::Pointer scalarImage = FloatScalarImageType::New();
  Testing::GetBlankImage<FloatScalarImageType>(scalarImage);

  // Make the left half of the image 0, and the right half 5
  itk::ImageRegionIterator<FloatScalarImageType> imageIterator(scalarImage, scalarImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < static_cast<int>(scalarImage->GetLargestPossibleRegion().GetSize()[0]/2))
      {
      imageIterator.Set(0);
      }
    else
      {
      imageIterator.Set(5);
      }

    ++imageIterator;
    }

  std::vector<itk::Offset<2> > offsets;
  itk::Offset<2> offset;
  offset.Fill(0);
  offsets.push_back(offset);
  offset.Fill(1);
  offsets.push_back(offset);

  itk::Size<2> patchSize;
  patchSize.Fill(10);

  std::cout << "Full patch different test." << std::endl;
  // Full patch is different
  {
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(0);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> sourcePatch(scalarImage, sourceRegion, true);

  itk::Index<2> targetCorner;
  targetCorner.Fill(scalarImage->GetLargestPossibleRegion().GetSize()[0]/2 + 4); // No magic about 4, just want a patch on the right side of the image
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> targetPatch(scalarImage, targetRegion, true);
  std::cout << "targetPatch: " << targetPatch << std::endl;

  PatchPair<FloatScalarImageType> patchPair(&sourcePatch, targetPatch);
  PatchDifferencePixelWiseSum<FloatScalarImageType, PixelDifference> scalar_patchDifferencePixelWiseSum;
  scalar_patchDifferencePixelWiseSum.SetImage(scalarImage);
  float difference = scalar_patchDifferencePixelWiseSum.Difference(patchPair, offsets);

  std::cout << "Number of pixels to compare: " << offsets.size() << std::endl;
  float correctDifference = offsets.size() * 5;
  if(difference != correctDifference)
    {
    std::stringstream ss;
    ss << "Difference " << difference << " does not match correctDifference " << correctDifference;
    throw std::runtime_error(ss.str());
    }
  }

  std::cout << "Identical patch test." << std::endl;
  // Patches are identical
  {
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(0);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> sourcePatch(scalarImage, sourceRegion, true);

  itk::Index<2> targetCorner;
  targetCorner.Fill(10); // No magic about 10, just want a patch not at (0,0) but still fully on the left side of the image
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> targetPatch(scalarImage, targetRegion, true);

  PatchPair<FloatScalarImageType> patchPair(&sourcePatch, targetPatch);
  PatchDifferencePixelWiseSum<FloatScalarImageType, PixelDifference> scalar_patchDifferencePixelWiseSum;
  scalar_patchDifferencePixelWiseSum.SetImage(scalarImage);
  float difference = scalar_patchDifferencePixelWiseSum.Difference(patchPair, offsets);

  float correctDifference = 0;
  if(difference != correctDifference)
    {
    std::stringstream ss;
    ss << "Difference " << difference << " does not match correctDifference " << correctDifference;
    throw std::runtime_error(ss.str());
    }
  }
}

void PartialPatchVectorComparison()
{
  std::cout << "PartialPatchVectorComparison()" << std::endl;

  const unsigned int dimension = 3;
  FloatVectorImageType::Pointer vectorImage = FloatVectorImageType::New();
  Testing::GetBlankImage<FloatVectorImageType>(vectorImage, dimension);

  // Make the left half of the image (0,0,0) and the right half (5,6,7)
  itk::ImageRegionIterator<FloatVectorImageType> imageIterator(vectorImage, vectorImage->GetLargestPossibleRegion());

  itk::VariableLengthVector<float> leftHalfPixel;
  leftHalfPixel.SetSize(dimension);
  leftHalfPixel.Fill(0);

  itk::VariableLengthVector<float> rightHalfPixel;
  rightHalfPixel.SetSize(dimension);
  rightHalfPixel[0] = 5;
  rightHalfPixel[1] = 6;
  rightHalfPixel[2] = 7;

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.GetIndex()[0] < static_cast<int>(vectorImage->GetLargestPossibleRegion().GetSize()[0]/2))
      {
      imageIterator.Set(leftHalfPixel);
      }
    else
      {
      imageIterator.Set(rightHalfPixel);
      }

    ++imageIterator;
    }

  itk::Size<2> patchSize;
  patchSize.Fill(10);

  // Full patches differ
  std::cout << "Full patch different test." << std::endl;
  {
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(0);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatVectorImageType> sourcePatch(vectorImage, sourceRegion, true);

  itk::Index<2> targetCorner;
  targetCorner.Fill(vectorImage->GetLargestPossibleRegion().GetSize()[0]/2 + 4); // No magic about 4, just want a patch on the right side of the image
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatVectorImageType> targetPatch(vectorImage, targetRegion, true);

  PatchPair<FloatVectorImageType> patchPair(&sourcePatch, targetPatch);
  PatchDifferencePixelWiseSum<FloatVectorImageType, PixelDifference> vector_patchDifferencePixelWiseSum;
  vector_patchDifferencePixelWiseSum.SetImage(vectorImage);
  float difference = vector_patchDifferencePixelWiseSum.Difference(patchPair);

  float correctDifference = targetRegion.GetNumberOfPixels() * 18; // 18 = 5+6+7, the sum of the elements of 'rightHalfPixel'
  if(difference != correctDifference)
    {
    std::stringstream ss;
    ss << "Difference " << difference << " does not match correctDifference " << correctDifference;
    throw std::runtime_error(ss.str());
    }
  }

  // Full patches identical
  std::cout << "Identical patch test." << std::endl;
  {
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(5);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatVectorImageType> sourcePatch(vectorImage, sourceRegion, true);

  itk::Index<2> targetCorner;
  targetCorner.Fill(5);
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatVectorImageType> targetPatch(vectorImage, targetRegion, true);

  PatchPair<FloatVectorImageType> patchPair(&sourcePatch, targetPatch);
  PatchDifferencePixelWiseSum<FloatVectorImageType, PixelDifference> vector_patchDifferencePixelWiseSum;
  vector_patchDifferencePixelWiseSum.SetImage(vectorImage);
  float difference = vector_patchDifferencePixelWiseSum.Difference(patchPair);

  float correctDifference = 0;
  if(difference != correctDifference)
    {
    std::stringstream ss;
    ss << "Difference " << difference << " does not match correctDifference " << correctDifference;
    throw std::runtime_error(ss.str());
    }
  }
}
