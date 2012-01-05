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

#ifndef SelfPatchCompare_H
#define SelfPatchCompare_H

/*
 * This class is for situations when you have a very large set of source patches
 * that are entirely valid, and you want to compare them all to a target patch
 * that is partially masked. It computes the linear offsets of the masked pixels
 * once, and then uses them to do all of the patch comparisons.
 */

// Custom
class CandidatePairs;

class Mask;

#include "DebugOutputs.h"
#include "PairDifferences.h"
#include "Patch.h"
#include "PatchPair.h"
#include "Types.h"

// ITK
#include "itkImageRegion.h"

// STL
#include <vector>

// Boost
#include <boost/function.hpp>

template <typename TImage, typename TPatchDifference>
class SelfPatchCompare : public DebugOutputs
{
public:
  //SelfPatchCompare(const FloatVectorImageType* image, const Mask* mask);
  SelfPatchCompare();

  // Compute the specified patch difference for all of the pairs.
  // Store the values in the PatchPair objects inside of the CandidatePairs object.
  void Compute();

  // Provide the image to work with.
  void SetImage(const TImage* const image);

  // Provide the mask to work with.
  void SetMask(const Mask* const mask);

  // Prepare to do some comparisons by finding all of the valid pixels in the target region
  void ComputeOffsets();

  void SetPairs(CandidatePairs* const pairs);

  void SetDifferenceType(const PairDifferences::PatchDifferenceTypes& differenceType);

protected:

  // These are the pixel offsets of the target region which we with to compare.
  std::vector<itk::Offset<2> > ValidTargetPatchPixelOffsets;

  // Provide the pairs of target/source patches. All target patches are exactly the same patch. The source regions are assumed to all be fully valid.
  // We modify the original data directory to add the computed values.
  CandidatePairs* Pairs;

  // This is the image from which to take the patches
  const TImage* Image;

  // This is the mask to check the validity of target pixels
  const Mask* MaskImage;

  PairDifferences::PatchDifferenceTypes DifferenceType;
};

#include "SelfPatchCompare.hxx"

#endif
