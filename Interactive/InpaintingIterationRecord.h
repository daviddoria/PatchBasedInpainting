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

  // Store the sets of pairs that were considered.
  std::vector<CandidatePairs*> PotentialPairSets;

  // Store the pair of patches that was actually used.
  PatchPair* UsedPatchPair;

  NamedITKImageCollection& GetImages();

  void AddImage(const NamedITKImage&, const bool display = false);

//   NamedITKImage GetImage(const unsigned int) const;
//   NamedITKImage GetImageByName(const std::string&) const;
//   unsigned int GetNumberOfImages() const;
// 
//   bool IsDisplayed(const unsigned int) const;
//   void SetDisplayed(const unsigned int, const bool displayed);

private:
  NamedITKImageCollection Images;

  //std::vector<bool> Display; // This vector is always the same length as the number of images.
};

#endif
