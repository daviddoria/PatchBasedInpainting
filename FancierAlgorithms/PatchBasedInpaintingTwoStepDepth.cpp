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

#include "PatchBasedInpaintingTwoStepDepth.h"


void PatchBasedInpaintingTwoStepDepth::FindBestPatch(CandidatePairs& candidatePairs, PatchPair& bestPatchPair)
{
  EnterFunction("PatchBasedInpaintingTwoStepDepth::FindBestPatch()");

  //std::cout << "FindBestPatch: There are " << candidatePairs.size() << " candidate pairs." << std::endl;
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(this->CompareImage);
  this->PatchCompare->SetMask(this->MaskImage);
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetMembershipImage(this->MembershipImage);

  this->PatchCompare->FunctionsToCompute.clear();
  this->PatchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchDepthDifference,this->PatchCompare,_1));
  this->PatchCompare->ComputeAllSourceDifferences();

  //std::cout << "FindBestPatch: Finished ComputeAllSourceDifferences()" << std::endl;

  std::sort(candidatePairs.begin(), candidatePairs.end(), SortByDifference(PatchPair::DepthDifference, PatchSortFunctor::ASCENDING));

  WriteImageOfScores(candidatePairs, this->CurrentOutputImage->GetLargestPossibleRegion(),
                     Helpers::GetSequentialFileName("Debug/ImageOfDepthScores", this->NumberOfCompletedIterations, "mha"));
  //candidatePairs.WriteDepthScoresToFile("candidateScores.txt");

  CandidatePairs goodDepthCandidatePairs;
  goodDepthCandidatePairs.CopyMetaOnly(candidatePairs);
  goodDepthCandidatePairs.insert(goodDepthCandidatePairs.end(), candidatePairs.begin(), candidatePairs.begin() + 1000);
  this->PatchCompare->SetPairs(&goodDepthCandidatePairs);

  //goodDepthCandidatePairs.WriteDepthScoresToFile("depthScores.txt");

  this->PatchCompare->FunctionsToCompute.clear();
  this->PatchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,this->PatchCompare,_1));
  this->PatchCompare->ComputeAllSourceDifferences();

  std::sort(goodDepthCandidatePairs.begin(), goodDepthCandidatePairs.end(), SortByDifference(PatchPair::AverageAbsoluteDifference, PatchSortFunctor::ASCENDING));
  WriteImageOfScores(goodDepthCandidatePairs, this->CurrentOutputImage->GetLargestPossibleRegion(),
                     Helpers::GetSequentialFileName("Debug/ImageOfTopColorScores", this->NumberOfCompletedIterations, "mha"));
  //std::cout << "Finished sorting " << candidatePairs.size() << " patches." << std::endl;

  // Return the result by reference.
  bestPatchPair = goodDepthCandidatePairs[0];

  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatch()." << std::endl;
  LeaveFunction("PatchBasedInpaintingTwoStepDepth::FindBestPatch()");
}
