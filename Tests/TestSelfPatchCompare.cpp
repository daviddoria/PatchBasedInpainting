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

#include "SelfPatchCompare.h"

#include "CandidatePairs.h"
#include "PatchDifferencePixelWiseSum.h"
#include "PixelDifference.h"

#include "Testing.h"
#include "Types.h"

#include "itkImageRegionIterator.h"

#include <iostream>

static void ScalarComparison();
static void VectorComparison();

int main(int argc, char*argv[])
{
  try
  {
    ScalarComparison();
    VectorComparison();
  }
  catch (std::runtime_error ex)
  {
    std::cerr << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void ScalarComparison()
{
  std::cout << "ScalarComparison()" << std::endl;
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

  Mask::Pointer mask = Mask::New();
  Testing::GetFullyValidMask(mask);

  const unsigned int patchRadius = 5;
  SourcePatchCollection sourcePatchCollection(mask, patchRadius);

  SourcePatchCollection::PatchContainer patches = sourcePatchCollection.FindSourcePatchesInRegion(mask->GetLargestPossibleRegion());

  sourcePatchCollection.AddPatches(patches);

  itk::Index<2> targetCorner;
  targetCorner.Fill(scalarImage->GetLargestPossibleRegion().GetSize()[0]/2 + 4); // No magic about 4, just want a patch on the right side of the image
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  Patch targetPatch(targetRegion);

  CandidatePairs candidatePairs(targetPatch);

  SelfPatchCompare<FloatScalarImageType> selfPatchCompare;
  selfPatchCompare.SetImage(scalarImage);
  selfPatchCompare.SetMask(mask);
  selfPatchCompare.SetDifferenceType(PairDifferences::SumPixelDifference);
  selfPatchCompare.SetPairs(&candidatePairs);
  selfPatchCompare.Compute<PatchDifferencePixelWiseSum<FloatScalarImageType, PixelDifference> >();

  for(CandidatePairs::Iterator pairsIterator = candidatePairs.begin(); pairsIterator != candidatePairs.end(); ++pairsIterator)
    {
    PatchPair currentPatchPair = *pairsIterator;
    if(currentPatchPair.GetTargetPatch() == targetPatch)
      {
      if(currentPatchPair.GetDifferences().GetDifferenceByType(PairDifferences::SumPixelDifference) != 0)
        {
        throw std::runtime_error("Difference of target patch with itself should be 0!");
        }
      }
    }

}

void VectorComparison()
{
#if 0
  std::cout << "VectorComparison()" << std::endl;

  const unsigned int dimension = 3;
  FloatVectorImageType::Pointer vectorImage = FloatVectorImageType::New();
  TestHelpers::GetBlankImage<FloatVectorImageType>(vectorImage, dimension);

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

  itk::Index<2> sourceCorner;
  sourceCorner.Fill(0);
  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  Patch sourcePatch(sourceRegion);

  itk::Index<2> targetCorner;
  targetCorner.Fill(vectorImage->GetLargestPossibleRegion().GetSize()[0]/2 + 4); // No magic about 4, just want a patch on the right side of the image
  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  Patch targetPatch(targetRegion);

  PatchPair patchPair(&sourcePatch, targetPatch);
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
#endif
}
