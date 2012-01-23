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

#include "ImagePatchPixelDescriptor.h"
#include "PatchPair.h"
#include "PatchPairDifferences.h"

int main(int argc, char*argv[])
{
  // Create a patch
  itk::Index<2> targetCorner;
  targetCorner.Fill(0);

  itk::Size<2> patchSize;
  patchSize.Fill(10);

  FloatScalarImageType::Pointer image = FloatScalarImageType::New();

  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> targetPatch(image, targetRegion);

  // Create another patch
  itk::Index<2> sourceCorner;
  sourceCorner.Fill(5);

  itk::ImageRegion<2> sourceRegion(sourceCorner, patchSize);
  ImagePatchPixelDescriptor<FloatScalarImageType> sourcePatch(image, sourceRegion);

  PatchPair<FloatScalarImageType> patchPair(&sourcePatch, targetPatch);

  // Compute the relative location of the source and target patch corners
  itk::Offset<2> correctTargetToSourceOffset = {{5, 5}};
  if(patchPair.GetTargetToSourceOffset() != correctTargetToSourceOffset)
    {
    std::cerr << "Incorrect TargetToSourceOffset! Should be " << correctTargetToSourceOffset
              << " but is " << patchPair.GetTargetToSourceOffset() << std::endl;
    return EXIT_FAILURE;
    }

  itk::Offset<2> correctSourceToTargetOffset = {{-5, -5}};
  if(patchPair.GetSourceToTargetOffset() != correctSourceToTargetOffset)
    {
    std::cerr << "Incorrect TargetToSourceOffset! Should be " << correctSourceToTargetOffset
              << " but is " << patchPair.GetSourceToTargetOffset() << std::endl;
    return EXIT_FAILURE;
    }

  if(*(patchPair.GetSourcePatch()) != sourcePatch)
    {
    std::cerr << "Source patch set or retrieved incorrectly!" << std::endl;
    return EXIT_FAILURE;
    }

  if(patchPair.GetTargetPatch() != targetPatch)
    {
    std::cerr << "Target patch set or retrieved incorrectly!" << std::endl;
    return EXIT_FAILURE;
    }

  patchPair.GetDifferences().SetDifferenceByType(PatchPairDifferences::AveragePixelDifference, 1.2f);
  if(patchPair.GetDifferences().GetDifferenceByType(PatchPairDifferences::AveragePixelDifference) != 1.2f)
    {
    std::cerr << "Difference was not set or retrieved correctly! Set as 1.2 but retrived as "
              << patchPair.GetDifferences().GetDifferenceByType(PatchPairDifferences::AveragePixelDifference) << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
