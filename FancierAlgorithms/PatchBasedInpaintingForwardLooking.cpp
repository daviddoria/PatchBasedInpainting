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

#include "PatchBasedInpaintingForwardLooking.h"


void PatchBasedInpaintingForwardLooking::FindBestPatch(CandidatePairs& candidatePairs, PatchPair& bestPatchPair)
{
  EnterFunction("PatchBasedInpaintingForwardLooking::FindBestPatch()");
  // This function returns the best PatchPair by reference

  //std::cout << "FindBestPatchLookAhead: There are " << this->SourcePatches.size() << " source patches at the beginning." << std::endl;

  // If this is not the first iteration, get the potential forward look patch candidates from the previous step
  if(this->NumberOfCompletedIterations > 0)
    {
    // Remove the patch candidate that was actually filled
    unsigned int idToRemove = 0;
    for(unsigned int forwardLookId = 0; forwardLookId < this->PotentialCandidatePairs.size(); ++forwardLookId)
      {
      if(PreviousIterationUsedPatchPair.TargetPatch.Region == this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region)
        {
        idToRemove = forwardLookId;
        break;
        }
      }
    this->PotentialCandidatePairs.erase(this->PotentialCandidatePairs.begin() + idToRemove);
    DebugMessage<unsigned int>("Removed forward look: ", idToRemove);
    }

  // We need to temporarily modify the priority image and boundary image without affecting the actual images, so we copy them.
  FloatScalarImageType::Pointer modifiedPriorityImage = FloatScalarImageType::New();
  Helpers::DeepCopy<FloatScalarImageType>(this->PriorityFunction->GetPriorityImage(), modifiedPriorityImage);

  UnsignedCharScalarImageType::Pointer modifiedBoundaryImage = UnsignedCharScalarImageType::New();
  Helpers::DeepCopy<UnsignedCharScalarImageType>(this->PriorityFunction->GetBoundaryImage(), modifiedBoundaryImage);

  // Blank all regions that are already look ahead patches.
  for(unsigned int forwardLookId = 0; forwardLookId < this->PotentialCandidatePairs.size(); ++forwardLookId)
    {
    Helpers::SetRegionToConstant<FloatScalarImageType>(modifiedPriorityImage, this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region, 0.0f);
    Helpers::SetRegionToConstant<UnsignedCharScalarImageType>(modifiedBoundaryImage, this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region, 0);
    }

  // Find the remaining number of patch candidates
  unsigned int numberOfNewPatchesToFind = this->MaxForwardLookPatches - this->PotentialCandidatePairs.size();
  for(unsigned int newPatchId = 0; newPatchId < numberOfNewPatchesToFind; ++newPatchId)
    {
    //std::cout << "FindBestPatchLookAhead: Start computing new patch " << newPatchId << std::endl;

    // If there are no boundary pixels, we can't find any more look ahead patches. This is not an error - near the end of the inpainting this will most likely occur. It is ok - we just don't have to compute as many forward look patches.
    if(Helpers::CountNonZeroPixels<UnsignedCharScalarImageType>(modifiedBoundaryImage) == 0)
      {
      //std::cout << "FindBestPatchLookAhead: There are no more boundary pixels!" << std::endl;
      break;
      }

    float highestPriority = 0.0f;
    itk::Index<2> pixelToFill = Helpers::FindHighestValueInMaskedRegion(modifiedPriorityImage, highestPriority, modifiedBoundaryImage);
    //std::cout << "Highest priority: " << highestPriority << std::endl;
    if(!this->MaskImage->HasHoleNeighbor(pixelToFill))
      {
      std::cerr << "pixelToFill " << pixelToFill << " does not have a hole neighbor - something is wrong!" << std::endl;
      std::cerr << "Mask value " << static_cast<unsigned int>(this->MaskImage->GetPixel(pixelToFill)) << std::endl;
      HelpersOutput::WriteImage<Mask>(this->MaskImage, "Debug/MaskImageError.mha");
      HelpersOutput::WriteImage<UnsignedCharScalarImageType>(modifiedBoundaryImage, "Debug/ModifiedBoundaryImageError.mha");
      HelpersOutput::WriteImage<FloatScalarImageType>(modifiedPriorityImage, "Debug/ModifiedPriorityImageError.mha");
      //std::cerr << "Boundary value " << static_cast<unsigned int>(this->BoundaryImage->GetPixel(pixelToFill)) << std::endl;

      std::stringstream ss;
      ss << "pixelToFill " << pixelToFill << " does not have a hole neighbor!";
      throw std::runtime_error(ss.str());
      }

    itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
    Patch targetPatch(targetRegion);

    CandidatePairs candidatePairs(targetPatch);
    candidatePairs.AddPairsFromPatches(this->SourcePatches);
    candidatePairs.Priority = highestPriority;

    PatchPair currentLookAheadBestPatchPair;
    //this->FindBestPatch(candidatePairs, currentLookAheadBestPatchPair);
    PatchBasedInpainting::FindBestPatch(candidatePairs, currentLookAheadBestPatchPair);

    // Keep only the number of top patches specified.
    //patchPairsSortedByContinuation.erase(patchPairsSortedByContinuation.begin() + this->NumberOfTopPatchesToSave, patchPairsSortedByContinuation.end());

    // Blank a region around the current potential patch to fill. This will ensure the next potential patch to fill is reasonably far away.
    Helpers::SetRegionToConstant<FloatScalarImageType>(modifiedPriorityImage, targetRegion, 0.0f);
    Helpers::SetRegionToConstant<UnsignedCharScalarImageType>(modifiedBoundaryImage, targetRegion, 0);

    //std::cout << "Sorted " << candidatePairs.size() << " candidatePairs." << std::endl;

    this->PotentialCandidatePairs.push_back(candidatePairs);
    //std::cout << "FindBestPatchLookAhead: Finished computing new patch " << newPatchId << std::endl;
    } // end forward look loop

  if(this->PotentialCandidatePairs.size() == 0)
    {
    throw std::runtime_error("Something is wrong - there are 0 forward look candidates!");
    }

  // Sort the forward look patches so that the highest priority sets are first in the vector (descending order).
  std::sort(this->PotentialCandidatePairs.rbegin(), this->PotentialCandidatePairs.rend(), SortByPriority);

  unsigned int bestForwardLookId = ComputeMinimumScoreLookAhead();

  unsigned int bestSourcePatchId = 0;
  //unsigned int bestSourcePatchId = GetRequiredHistogramIntersection(bestForwardLookId);

  std::cout << "Best pair found to be " << bestForwardLookId << " " << bestSourcePatchId << std::endl;

  // Return the result by reference.
  bestPatchPair = this->PotentialCandidatePairs[bestForwardLookId][bestSourcePatchId];

//   std::cout << "Used patch " << sourcePatchId - 1 << std::endl;

  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end." << std::endl;
  LeaveFunction("PatchBasedInpaintingForwardLooking::FindBestPatch()");
}


unsigned int PatchBasedInpaintingForwardLooking::ComputeMinimumScoreLookAhead()
{
  EnterFunction("ComputeMinimumScoreLookAhead()");
  // Choose the look ahead with the lowest score to actually fill rather than simply returning the best source patch of the first look ahead target patch.
  float lowestScore = std::numeric_limits< float >::max();
  unsigned int lowestLookAhead = 0;
  for(unsigned int i = 0; i < this->PotentialCandidatePairs.size(); ++i)
    {
    if(this->PotentialCandidatePairs[i][0].DifferenceMap[PatchPair::AverageAbsoluteDifference] < lowestScore)
      {
      lowestScore = this->PotentialCandidatePairs[i][0].DifferenceMap[PatchPair::AverageAbsoluteDifference];
      lowestLookAhead = i;
      }
    }
  LeaveFunction("ComputeMinimumScoreLookAhead()");
  return lowestLookAhead;
}

void PatchBasedInpaintingForwardLooking::SetMaxForwardLookPatches(const unsigned int numberOfPatches)
{
  this->MaxForwardLookPatches = numberOfPatches;
}
