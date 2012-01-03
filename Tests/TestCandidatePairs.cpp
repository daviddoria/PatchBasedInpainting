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

#include "CandidatePairs.h"
#include "Mask.h"
#include "SourcePatchCollection.h"

static void CreateMask(Mask* mask);

int main(int argc, char*argv[])
{
  // Setup target patch
  itk::Index<2> targetCorner;
  targetCorner.Fill(0);

  itk::Size<2> patchSize;
  patchSize.Fill(10);

  itk::ImageRegion<2> targetRegion(targetCorner, patchSize);
  Patch targetPatch(targetRegion);

  Mask::Pointer mask = Mask::New();
  CreateMask(mask);
  const unsigned int patchRadius = 5;
  SourcePatchCollection sourcePatchCollection(mask, patchRadius);

  SourcePatchCollection::PatchContainer patches = sourcePatchCollection.FindSourcePatchesInRegion(mask->GetLargestPossibleRegion());

  sourcePatchCollection.AddPatches(patches);
  
  CandidatePairs candidatePairs(targetPatch);

  if(candidatePairs.GetTargetPatch() != targetPatch)
    {
    std::cerr << "TargetPatch not set or retrieved correctly! Retrieved as " << candidatePairs.GetTargetPatch()
              << " but should be " << targetPatch << std::endl;
    return EXIT_FAILURE;
    }

  candidatePairs.AddSourcePatches(sourcePatchCollection);

  unsigned int counter = 0;

  for(CandidatePairs::Iterator pairsIterator = candidatePairs.begin(); pairsIterator != candidatePairs.end(); ++pairsIterator)
    {
    counter++;
    }

  if(counter != candidatePairs.GetNumberOfSourcePatches())
    {
    std::cerr << "Should have iterated over " << candidatePairs.GetNumberOfSourcePatches() << " but only reached " << counter << std::endl;
    return EXIT_FAILURE;
    }

  std::vector<std::shared_ptr<PatchPair> > pairs = candidatePairs.GetPatchPairs();
  if(pairs.size() != candidatePairs.GetNumberOfSourcePatches())
    {
    std::cerr << "GetPatchPairs() failed!" << std::endl;
    return EXIT_FAILURE;
    }

  // Test priority
  candidatePairs.SetPriority(1.2);
  if(candidatePairs.GetPriority() != 1.2)
    {
    std::cerr << "SetPriority or GetPriority failed!" << std::endl;
    return EXIT_FAILURE;
    }
  
  const PatchPair patchPair = candidatePairs.GetPair(0);

  const Patch* sourcePatch = candidatePairs.GetSourcePatch(0);
  
  Patch retrievedTargetPatch = candidatePairs.GetTargetPatch();
  if(retrievedTargetPatch != targetPatch)
    {
    std::cerr << "Target patch set or retrieved improperly!" << std::endl;
    return EXIT_FAILURE;
    }

  // TODO: Test these
  // void Sort(SortFunctorWrapper sortFunctor);
  // void Combine(const CandidatePairs& pairs);

  return EXIT_SUCCESS;
}


void CreateMask(Mask* mask)
{
  itk::Index<2> maskCorner;
  maskCorner.Fill(0);

  itk::Size<2> maskSize;
  maskSize.Fill(100);

  itk::ImageRegion<2> maskRegion(maskCorner, maskSize);

  mask->SetRegions(maskRegion);
  mask->Allocate();

  unsigned int indeterminateValue = (static_cast<unsigned int>(mask->GetHoleValue()) + static_cast<unsigned int>(mask->GetValidValue()))/2;

  // Make the left half of the mask the hole, the center indeterminate, and the right half valid.
  for(unsigned int column = 0; column < maskSize[0]; ++column)
    {
    for(unsigned int row = 0; row < maskSize[1]; ++row)
      {
      itk::Index<2> currentIndex;
      currentIndex[0] = column;
      currentIndex[1] = row;
      if(column < maskSize[0] / 3)
        {
        mask->SetPixel(currentIndex, mask->GetHoleValue());
        }
      else if(column < 2*maskSize[0]/3)
        {
        mask->SetPixel(currentIndex, indeterminateValue);
        }
      else
        {
        mask->SetPixel(currentIndex, mask->GetValidValue());
        }
      }
    }
}
