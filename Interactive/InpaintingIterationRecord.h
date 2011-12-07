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

#ifndef InpaintingIterationRecord_H
#define InpaintingIterationRecord_H

#include "CandidatePairs.h"
#include "NamedITKImageCollection.h"
#include "Mask.h"
#include "Types.h"

// This class allows us to record iterations of the inpainting procedure.
// It stores the image and mask AFTER the ith iteration is complete.
// For this reason, the PotentialPairSets are the pairs that were considered
// BEFORE moving to this state.
class InpaintingIterationRecord
{
public:
  InpaintingIterationRecord();
  
//   FloatVectorImageType::Pointer Image;
//   Mask::Pointer MaskImage;
//   UnsignedCharScalarImageType::Pointer Boundary;
//   FloatScalarImageType::Pointer Priority;
  NamedITKImageCollection Images;

  // Store the sets of pairs that were considered.
  std::vector<CandidatePairs> PotentialPairSets;

  // Store the pairs of patches that were actually used.
  PatchPair UsedPatchPair;
  
};

#endif
