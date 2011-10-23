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

CandidatePairs::CandidatePairs(const Patch& targetPatch)
{
  this->TargetPatch = targetPatch;

  this->Priority = 0.0f;
}

void CandidatePairs::AddCandidatePair(const PatchPair& patchPair)
{
  if(patchPair.TargetPatch.Region == this->TargetPatch.Region)
    {
    this->push_back(patchPair);
    }
  else
    {
    std::cerr << "Trying to add a pair to the list of candidate pairs that does not match the target patch!" << std::endl;
    }
}

void CandidatePairs::AddPairsFromPatches(const std::vector<Patch>& patches)
{
  for(unsigned int i = 0; i < patches.size(); ++i)
    {
    PatchPair patchPair;
    patchPair.TargetPatch = this->TargetPatch;
    patchPair.SourcePatch = patches[i];
    this->push_back(patchPair);
    }
  std::cout << "Added " << patches.size() << " pairs." << std::endl;
}

void CandidatePairs::CopyFrom(const std::vector<PatchPair>& v)
{
  this->clear();
  for(unsigned int i = 0; i < v.size(); ++i)
    {
    this->push_back(v[i]);
    }
}

bool SortByPriority(const CandidatePairs& candidatePairs1, const CandidatePairs& candidatePairs2)
{
  return (candidatePairs1.Priority < candidatePairs2.Priority);
}
