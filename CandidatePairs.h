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

#ifndef CANDIDATEPAIRS_H
#define CANDIDATEPAIRS_H

#include "PatchPair.h"

#include <vector>

// This class stores a target patch and a list of the top N (user specified) pairs of patches based on the user specified comparison criteria.
// All patch pairs must have the same target patch.

class CandidatePairs : public std::vector<PatchPair>
{
public:
  CandidatePairs(){} // This is so that we can construct a CandidatePairs to be filled by an accessor.
  
  CandidatePairs(const Patch& targetPatch);
  
  void AddCandidatePair(const PatchPair& patchPair);

  Patch TargetPatch;
  
  void AddPairsFromPatches(const std::vector<Patch>& patches);

  void AddPairFromPatch(const Patch& patch);

  void CopyFrom(const std::vector<PatchPair>& v);

  float Priority;

  void InvalidateAll();
  
  void Combine(CandidatePairs& pairs);
};

bool SortByPriority(const CandidatePairs& candidatePairs1, const CandidatePairs& candidatePairs2);

#endif
