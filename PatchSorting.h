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

struct PatchSortFunctor
{
  virtual bool operator()(const PatchPair &T1, const PatchPair &T2) = 0;

  boost::function<bool (const PatchPair& , const PatchPair& )> PatchSortFunction;
};

// Sorting functions
bool SortByAverageSquaredDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByAverageAbsoluteDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryGradientDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryIsophoteAngleDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryIsophoteStrengthDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryPixelDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByDepthDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByColorDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByTotalScore(const PatchPair& pair1, const PatchPair& pair2);

struct SortByDepthAndColor
{
  bool operator()(const PatchPair& pair1, const PatchPair& pair2);
  SortByDepthAndColor() : DepthColorLambda(0.5f){}
  float DepthColorLambda;
};

#endif
