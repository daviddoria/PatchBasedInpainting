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

#include "PatchBasedInpaintingScaleConsistent.h"

void PatchBasedInpaintingScaleConsistent::FindBestPatch(CandidatePairs& candidatePairs, PatchPair& bestPatchPair)
{
  //std::cout << "FindBestPatchScaleConsistent: There are " << this->SourcePatches.size() << " source patches at the beginning." << std::endl;
  EnterFunction("FindBestPatchScaleConsistent()");

  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(this->BlurredImage);
  this->PatchCompare->SetMask(this->MaskImage);
  this->PatchCompare->SetMembershipImage(this->MembershipImage);
  this->PatchCompare->ComputeAllSourceDifferences();

  std::sort(candidatePairs.begin(), candidatePairs.end(), SortFunctorWrapper(this->PatchSortFunction));
  //std::cout << "Blurred score for pair 0: " << candidatePairs[0].GetAverageAbsoluteDifference() << std::endl;
  candidatePairs.InvalidateAll();

  std::vector<float> blurredScores(candidatePairs.size());
  for(unsigned int i = 0; i < candidatePairs.size(); ++i)
    {
    blurredScores[i] = candidatePairs[i].DifferenceMap[PatchPair::AverageAbsoluteDifference];
    }

  // Create a temporary image to fill for now, but we might not actually end up filling this patch.
  FloatVectorImageType::Pointer tempImage = FloatVectorImageType::New();
  Helpers::DeepCopy<FloatVectorImageType>(this->CurrentOutputImage, tempImage);
  // Fill the detailed image hole with a part of the blurred image
  Helpers::CopySourcePatchIntoHoleOfTargetRegion<FloatVectorImageType>(this->BlurredImage, tempImage, this->MaskImage, candidatePairs[0].SourcePatch.Region, candidatePairs[0].TargetPatch.Region);

  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(tempImage);
  this->PatchCompare->SetMask(this->MaskImage);
  this->PatchCompare->SetMembershipImage(this->MembershipImage);
  this->PatchCompare->ComputeAllSourceAndTargetDifferences();

  //std::cout << "Detailed score for pair 0: " << candidatePairs[0].GetAverageAbsoluteDifference() << std::endl;

  for(unsigned int i = 0; i < candidatePairs.size(); ++i)
    {
    candidatePairs[i].DifferenceMap[PatchPair::AverageAbsoluteDifference] = blurredScores[i] + candidatePairs[i].DifferenceMap[PatchPair::AverageAbsoluteDifference];
    }

  std::cout << "Total score for pair 0: " << candidatePairs[0].DifferenceMap[PatchPair::AverageAbsoluteDifference] << std::endl;
  std::sort(candidatePairs.begin(), candidatePairs.end(), SortFunctorWrapper(this->PatchSortFunction));

  // Return the result by reference.
  bestPatchPair = candidatePairs[0];

  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatchScaleConsistent()." << std::endl;
  LeaveFunction("FindBestPatchScaleConsistent()");
}
