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

#ifndef PATCHSORTING_H
#define PATCHSORTING_H

#include "CandidatePairs.h"
#include "PatchPairDifferences.h"
#include "PatchPair.h"

//   void Sort(SortFunctorWrapper sortFunctor);
//
//   bool SortByPriority(const CandidatePairs& candidatePairs1, const CandidatePairs& candidatePairs2);


// Non-member functions
// bool SortByPriority(const CandidatePairs& candidatePairs1, const CandidatePairs& candidatePairs2)
// {
//   return (candidatePairs1.GetPriority() < candidatePairs2.GetPriority());
// }

// This is a pure virtual functor that defines the required interface.
struct PatchSortFunctor
{
  enum SortOrderEnum {ASCENDING, DESCENDING};
  PatchSortFunctor(const PatchPairDifferences::PatchDifferenceTypes differenceType, const SortOrderEnum sortOrder);
  virtual bool operator()(const PatchPair &T1, const PatchPair &T2) = 0;
  PatchPairDifferences::PatchDifferenceTypes DifferenceType;
  SortOrderEnum SortOrder;
};

// This class is necessary to pass a subclass of a pure virtual functor to sort()
// PatchSortFunctor* test = new SortByAverageSquaredDifference;
// std::sort(testVector.begin(), testVector.end(), SortFunctorWrapper(test));
struct SortFunctorWrapper
{
  SortFunctorWrapper(PatchSortFunctor* func) : func_(func) {}
  bool operator()(const PatchPair &T1, const PatchPair &T2);
  bool operator()(PatchPair* const pair1, PatchPair* const pair2);
  bool operator()(const std::shared_ptr<PatchPair>& pair1, const std::shared_ptr<PatchPair>& pair2);

  PatchSortFunctor* func_;
};

struct SortByDifference : public PatchSortFunctor
{
  SortByDifference(PatchPairDifferences::PatchDifferenceTypes differenceType, const SortOrderEnum sortOrder) : PatchSortFunctor(differenceType, sortOrder){}
  bool operator()(const PatchPair& pair1, const PatchPair& pair2);
};

#endif
