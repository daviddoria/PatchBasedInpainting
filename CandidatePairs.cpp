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

#include <fstream>

CandidatePairs::CandidatePairs(const Patch& targetPatch)
{
  this->TargetPatch = targetPatch;

  this->Priority = 0.0f;
}

void CandidatePairs::CopyFrom(const CandidatePairs& pairs)
{
  this->TargetPatch = pairs.TargetPatch;
  this->Priority = pairs.Priority;
  this->clear();
  for(unsigned int i = 0; i < pairs.size(); ++i)
    {
    this->push_back(pairs[i]);
    }
}

void CandidatePairs::CopyMetaOnly(const CandidatePairs& pairs)
{
  this->TargetPatch = pairs.TargetPatch;
  this->Priority = pairs.Priority;
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
  //std::cout << "AddPairsFromPatches(): Added " << patches.size() << " pairs." << std::endl;
}

void CandidatePairs::AddPairFromPatch(const Patch& patch)
{
  PatchPair patchPair;
  patchPair.TargetPatch = this->TargetPatch;
  patchPair.SourcePatch = patch;
  this->push_back(patchPair);
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

void CandidatePairs::InvalidateAll()
{
  for(unsigned int i = 0; i < (*this).size(); ++i)
    {
    (*this)[i].SetValidAverageSquaredDifference(false);
    (*this)[i].SetValidAverageAbsoluteDifference(false);
    (*this)[i].SetValidBoundaryGradientDifference(false);
    (*this)[i].SetValidBoundaryPixelDifference(false);
    (*this)[i].SetValidBoundaryIsophoteAngleDifference(false);
    (*this)[i].SetValidBoundaryIsophoteStrengthDifference(false);
    }
}

void CandidatePairs::Combine(CandidatePairs& pairs)
{
  if(pairs.TargetPatch.Region != this->TargetPatch.Region)
    {
    std::cerr << "Cannot combine CandidatePairs that are not of the same TargetPatch!" << std::endl;
    exit(-1);
    }
  for(unsigned int i = 0; i < pairs.size(); ++i)
    {
    this->push_back(pairs[i]);
    }
}

void CandidatePairs::WriteDepthScoresToFile(const std::string& fileName)
{
  std::ofstream fout(fileName.c_str());
  for(unsigned int i = 0; i < this->size(); ++i)
    {
    fout << (*this)[i].GetDepthDifference() << std::endl;
    }
    
  fout.close();
}
