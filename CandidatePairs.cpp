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
#include "PatchPair.h"

#include <memory>

CandidatePairs::CandidatePairs(const Patch& targetPatch) : Priority(0.0f), TargetPatch(targetPatch)
{

}



std::vector<itk::Offset<2> > CandidatePairs::ComputeOffsets(const Mask* const mask)
{
  // This function computes the list of offsets that are from the source region of the target patch.
  std::vector<itk::Offset<2> > offsets;
  
  // Iterate over the target region of the mask. Add the linear offset of valid pixels to the offsets to be used later in the comparison.
  itk::ImageRegionConstIterator<Mask> maskIterator(mask, this->GetTargetPatch().GetRegion());
  itk::Index<2> targetCorner = this->GetTargetPatch().GetCorner();
  while(!maskIterator.IsAtEnd())
    {
    if(mask->IsValid(maskIterator.GetIndex()))
      {
      // The ComputeOffset function returns the linear index of the pixel.
      // To compute the memory address of the pixel, we must multiply by the number of components per pixel.
      itk::Offset<2> offset = maskIterator.GetIndex() - targetCorner;

      //std::cout << "Using offset: " << offset << std::endl;
      offsets.push_back(offset); // We have to multiply the linear offset by the number of components per pixel for the VectorImage type
      }

    ++maskIterator;
    }
  return offsets;
}
void CandidatePairs::Sort(const PatchPairDifferences::PatchPairDifferenceTypes sortBy, const SortOrderEnum ordering)
{
  PairSortFunctor sortFunctor(sortBy);
  std::sort(this->PatchPairs.begin(), this->PatchPairs.end(), sortFunctor);
}

void CandidatePairs::AddSourcePatches(const SourcePatchCollection& patches)
{
  for(SourcePatchCollection::Iterator patchIterator = patches.begin(); patchIterator != patches.end(); ++patchIterator)
    {
    const Patch* sourcePatch = &(*patchIterator);
    std::shared_ptr<PatchPair> patchPair = std::shared_ptr<PatchPair>(new PatchPair(sourcePatch, this->TargetPatch));
    this->PatchPairs.push_back(patchPair);
    }
}

CandidatePairs::Iterator CandidatePairs::begin()
{
  return this->PatchPairs.begin();
}

CandidatePairs::Iterator CandidatePairs::end()
{
  return this->PatchPairs.end();
}

CandidatePairs::ConstIterator CandidatePairs::begin() const
{
  return this->PatchPairs.begin();
}

CandidatePairs::ConstIterator CandidatePairs::end() const
{
  return this->PatchPairs.end();
}

// const PatchPair& CandidatePairs::GetPair(const unsigned int pairId) const
// {
//   return *(this->PatchPairs[pairId]);
// }
// 
// PatchPair& CandidatePairs::GetPair(const unsigned int pairId)
// {
//   return *(this->PatchPairs[pairId]);
// }

// const Patch* const CandidatePairs::GetSourcePatch(const unsigned int pairId) const
// {
//   return this->PatchPairs[pairId]->GetSourcePatch();
// }

const Patch& CandidatePairs::GetTargetPatch() const
{
  return this->TargetPatch;
}

unsigned int CandidatePairs::GetNumberOfSourcePatches() const
{
  return this->PatchPairs.size();
}

// void CandidatePairs::AddCandidatePair(const PatchPair& patchPair)
// {
//   assert(patchPair.GetTargetPatch() == this->PatchPairs[0]->GetTargetPatch());
// 
//   this->PatchPairs.push_back(std::shared_ptr<PatchPair>(new PatchPair(patchPair)));
// }

float CandidatePairs::GetPriority() const
{
  return this->Priority;
}

void CandidatePairs::SetPriority(const float priority)
{
  this->Priority = priority;
}

// std::vector<PatchPair> CandidatePairs::GetAllPairs() const
// {
//   std::vector<PatchPair> pairs;
//   for(unsigned int patchId = 0; patchId < this->PatchPairs.size(); ++patchId)
//     {
//     pairs.push_back(PatchPair(this->GetTargetPatch(), this->GetSourcePatch(patchId)));
//     }
//   return pairs;
// }

std::vector<std::shared_ptr<PatchPair> > CandidatePairs::GetPatchPairs()
{
  return this->PatchPairs;
}

void CandidatePairs::Combine(const CandidatePairs& candidatePairs)
{
  if(candidatePairs.GetTargetPatch() != this->GetTargetPatch())
    {
    throw std::runtime_error("Cannot combine CandidatePairs that are not of the same TargetPatch!");
    }
//   for(unsigned int patchId = 0; patchId < candidatePairs.GetNumberOfSourcePatches(); ++patchId)
//     {
//     this->PatchPairs.push_back(std::shared_ptr<PatchPair>(new PatchPair(candidatePairs.GetPair(patchId))));
//     }
  for(ConstIterator patchIterator = candidatePairs.begin(); patchIterator != candidatePairs.end(); ++patchIterator)
    {
    this->PatchPairs.push_back(std::shared_ptr<PatchPair>(new PatchPair(*patchIterator)));
    }
}


// void CandidatePairs::OutputPairs(const std::vector<PatchPair>& patchPairs, const std::string& filename)
// {
//   std::ofstream fout(filename.c_str());
// 
//   for(unsigned int i = 0; i < patchPairs.size(); ++i)
//     {
//     fout << "Potential patch " << i << ": " << std::endl
//         << "target index: " << patchPairs[i].TargetPatch.Region.GetIndex() << std::endl;
//         //<< "ssd score: " << patchPairs[i].GetAverageSSD() << std::endl;
//         //<< "histogram score: " << patchPairs[i].HistogramDifference << std::endl;
//     }
// 
//   fout.close();
// }
