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

#include "PatchPair.h"

PatchPair::PatchPair(const Patch* const sourcePatch, const Patch& targetPatch) : SourcePatch(sourcePatch), TargetPatch(targetPatch)
{

}

// PatchPair& PatchPair::operator= (const PatchPair& other)
// {
//   if (this != &other)
//   {
//     this->SourcePatch = other.SourcePatch;
//   }
// 
//   return *this;
// }

PatchPairDifferences& PatchPair::GetDifferences()
{
  return Differences;
}

const PatchPairDifferences& PatchPair::GetDifferences() const
{
  return Differences;
}

itk::Offset<2> PatchPair::GetTargetToSourceOffset() const
{
  return this->SourcePatch->GetRegion().GetIndex() - this->TargetPatch.GetRegion().GetIndex();
}

itk::Offset<2> PatchPair::GetSourceToTargetOffset() const
{
  return this->TargetPatch.GetRegion().GetIndex() - this->SourcePatch->GetRegion().GetIndex();
}

const Patch* PatchPair::GetSourcePatch() const
{
  return this->SourcePatch;
}

const Patch& PatchPair::GetTargetPatch() const
{
  return this->TargetPatch;
}
/*
float PatchPair::GetDepthAndColorDifference() const
{
  DifferenceMapType::const_iterator colorIter = this->DifferenceMap.find(ColorDifference);

  if(colorIter == this->DifferenceMap.end())
    {
    throw std::runtime_error("Could not compute GetDepthAndColorDifference, ColorDifference not found.");
    }

  DifferenceMapType::const_iterator depthIter = this->DifferenceMap.find(DepthDifference);

  if(depthIter == this->DifferenceMap.end())
    {
    throw std::runtime_error("Could not compute GetDepthAndColorDifference, DepthDifference not found.");
    }

  return ComputeDepthAndColorDifference(this->DifferenceMap.find(DepthDifference)->second, this->DifferenceMap.find(ColorDifference)->second);
}*/

bool PairSortFunctor::operator()(const PatchPair& pair1, const PatchPair& pair2)
{
//   if(SortOrder == ASCENDING)
//     {
//     return (pair1.GetDifferences().GetDifferenceByType(DifferenceType) < pair2.GetDifferences().GetDifferenceByType(DifferenceType));
//     }
//   else
//     {
//     return !(pair1.GetDifferences().GetDifferenceByType(DifferenceType) < pair2.GetDifferences().GetDifferenceByType(DifferenceType));
//     }
  return !(pair1.GetDifferences().GetDifferenceByType(SortBy) < pair2.GetDifferences().GetDifferenceByType(SortBy));
}

bool PairSortFunctor::operator()(const std::shared_ptr<PatchPair>& pair1, const std::shared_ptr<PatchPair>& pair2)
{
  return !(pair1->GetDifferences().GetDifferenceByType(SortBy) < pair2->GetDifferences().GetDifferenceByType(SortBy));
}

