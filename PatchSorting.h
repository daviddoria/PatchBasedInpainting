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

#include "PatchPair.h"

// This is a pure virtual functor that defines the required interface.
struct PatchSortFunctor
{
  PatchSortFunctor(PatchPair::PatchDifferenceTypes differenceType);
  virtual bool operator()(const PatchPair &T1, const PatchPair &T2) = 0;
  PatchPair::PatchDifferenceTypes DifferenceType;
};

// This class is necessary to pass a subclass of a pure virtual functor to sort()
// PatchSortFunctor* test = new SortByAverageSquaredDifference;
// std::sort(testVector.begin(), testVector.end(), SortFunctorWrapper(test));
struct SortFunctorWrapper
{
    SortFunctorWrapper(PatchSortFunctor* func) : func_(func) {}
    bool operator()(const PatchPair &T1, const PatchPair &T2)
    {
        return (*func_)(T1, T2);
    }
    PatchSortFunctor* func_;
};

struct SortByDifference : public PatchSortFunctor
{
  SortByDifference(PatchPair::PatchDifferenceTypes differenceType) : PatchSortFunctor(differenceType){}
  bool operator()(const PatchPair& pair1, const PatchPair& pair2);
};

// struct SortByTotalScore : public PatchSortFunctor
// {
//   SortByTotalScore(PatchPair::PatchDifferenceTypes differenceType) : PatchSortFunctor(differenceType){}
//   bool operator()(const PatchPair& pair1, const PatchPair& pair2);
// };

struct SortByDepthAndColor : public PatchSortFunctor
{
  bool operator()(const PatchPair& pair1, const PatchPair& pair2);
  SortByDepthAndColor(PatchPair::PatchDifferenceTypes differenceType) : PatchSortFunctor(differenceType), DepthColorLambda(0.5f){}
  float DepthColorLambda;
};

#endif
