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

// This class stores a target patch (a pointer to one of the source patches in PatchBasedInpainting)
// and a list of N (user specified) source patches (also pointers to source patches in PatchBasedInpainting)

class CandidatePairs
{
public:
  CandidatePairs();

  void AddCandidatePair(const PatchPair& patchPair);

  void AddSourcePatches(const std::vector<Patch*>& patches);

  void AddSourcePatch(const Patch* patch);

  void InvalidateAll();

  void Combine(CandidatePairs& pairs);

  //void WriteDepthScoresToFile(const std::string& fileName);

  //std::vector<PatchPair> GetAllPairs() const;

  std::vector<std::shared_ptr<PatchPair> > GetPatchPairs();

  PatchPair GetPair(const unsigned int pairId) const;
  const Patch* GetSourcePatch(const unsigned int pairId) const;
  const Patch* GetTargetPatch() const;
  float GetPriority() const;
  void SetPriority(const float priority);
  unsigned int GetNumberOfSourcePatches() const;

private:
  std::vector<std::shared_ptr<PatchPair> > PatchPairs;

  float Priority;
};

bool SortByPriority(const CandidatePairs& candidatePairs1, const CandidatePairs& candidatePairs2);

#endif
