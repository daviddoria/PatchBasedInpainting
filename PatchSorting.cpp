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

#include "PatchSorting.h"

bool SortFunctorWrapper::operator()(const PatchPair &T1, const PatchPair &T2)
{
  return (*func_)(T1, T2);
}

bool SortFunctorWrapper::operator()(PatchPair* const pair1, PatchPair* const pair2)
{
  return (*func_)(*pair1, *pair2);
}

bool SortFunctorWrapper::operator()(const std::shared_ptr<PatchPair>& pair1, const std::shared_ptr<PatchPair>& pair2)
{
  return (*func_)(*pair1, *pair2);
}

PatchSortFunctor::PatchSortFunctor(const PairDifferences::PatchDifferenceTypes differenceType, const SortOrderEnum sortOrder) :
DifferenceType(differenceType), SortOrder(sortOrder)
{

}

bool SortByDifference::operator()(const PatchPair& pair1, const PatchPair& pair2)
{
  if(SortOrder == ASCENDING)
    {
    return (pair1.GetDifferences().GetDifferenceByType(DifferenceType) < pair2.GetDifferences().GetDifferenceByType(DifferenceType));
    }
  else
    {
    return !(pair1.GetDifferences().GetDifferenceByType(DifferenceType) < pair2.GetDifferences().GetDifferenceByType(DifferenceType));
    }
}



// bool SortByTotalScore::operator()(const PatchPair& pair1, const PatchPair& pair2)
// {
//   return (pair1.GetTotalScore() < pair2.GetTotalScore());
// }
/*
bool SortByDepthAndColor::operator()(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetDepthAndColorDifference() < pair2.GetDepthAndColorDifference());
}*/
