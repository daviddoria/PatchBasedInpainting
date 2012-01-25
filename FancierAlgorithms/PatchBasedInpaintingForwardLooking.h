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

#ifndef PatchBasedInpaintingForwardLooking_h
#define PatchBasedInpaintingForwardLooking_h

#include "PatchBasedInpainting.h"

// This is a new idea to try to fill several patches and return the best pair.
// Note that if the number of look ahead patches is 1, this is exactly the same as not looking ahead.
//void FindBestPatchLookAhead(PatchPair& bestPatchPair);

class PatchBasedInpaintingForwardLooking : public PatchBasedInpainting
{

public:
  void FindBestPatch(CandidatePairs& candidatePairs, PatchPair& bestPatchPair);


  // Specify the maximum number of top candidate patches to consider. Near the end of the inpainting there may not be this many viable patches,
  // that is why we set the max instead of the absolute number of patches.
  void SetMaxForwardLookPatches(const unsigned int);

  void SetNumberOfTopPatchesToSave(const unsigned int);

protected:

  unsigned int ComputeMinimumScoreLookAhead();

  // The maximum number of patch pairs to examine in deciding which one to actually fill.
  // The number compared could actually be less than this near the end of the inpainting because there may
  // not be enough non-zero priority values outside of one patch region.
  unsigned int MaxForwardLookPatches;
};

#endif
