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
#include "CandidatePairs.h"
#include "Helpers.h"
#include "Mask.h"
#include "Patch.h"
#include "Types.h"

// ITK
#include "itkImageRegion.h"

// STL
#include <vector>

// Boost
#include <boost/function.hpp>

class SelfPatchCompare
{
  
public:
  SelfPatchCompare();
  
  // Compute the SSD for all of the pairs. Store the values in the PatchPair objects inside of the CandidatePairs object.
  void ComputeAllSourceDifferences();
  
  void ComputeAllSourceAndTargetDifferences();
  
  // Provide the image to work with.
  void SetImage(const FloatVectorImageType::Pointer);

  // Provide the mask to work with.
  void SetMask(const Mask::Pointer mask);

  float SlowDifference(const Patch& sourcePatch);
  float PatchDifferenceManual(const Patch& sourcePatch);
  
  float PatchAverageAbsoluteSourceDifference(const Patch& sourcePatch);
  //float PatchAverageSquaredDifference(const Patch& sourcePatch);
  
  // This function loops through FunctionsToCompute and calls them all.
  void SetPatchAllDifferences(PatchPair& patchPair);
  
  // This is the potential list of FunctionsToCompute
  void SetPatchAverageAbsoluteSourceDifference(PatchPair& patchPair);
  void SetPatchAverageAbsoluteFullDifference(PatchPair& patchPair);
  //void SetPatchAverageSquaredDifference(PatchPair& patchPair);
  void SetPatchDepthDifference(PatchPair& patchPair);
  void SetPatchColorDifference(PatchPair& patchPair);
  
  float PatchSourceDifferenceBoundary(const Patch& sourcePatch);

  // Prepare to do some comparisons by finding all of the valid pixels in the target region
  void ComputeOffsets();
  
  void SetPairs(CandidatePairs* pairs);
  
  void SetNumberOfComponentsPerPixel(const unsigned int numberOfComponentsPerPixel);
  
  std::vector<boost::function< void(PatchPair&) > > FunctionsToCompute;
  
protected:
  // If a channel of one pixel was white (255) and the corresponding channel of the other pixel
  // was black (0), the difference would be 255, so the difference squared would be 255*255
  static const float MaxColorDifference = 255*255;
  
  // These are the offsets of the target region which we with to compare
  std::vector<FloatVectorImageType::OffsetValueType> ValidOffsets;

  // This is the target region we wish to compare. It may be partially invalid.
  //Patch TargetPatch;

  // Provide the pairs of target/source patches. All target patches are exactly the same patch. The source regions are assumed to all be fully valid.
  // We modify the original data directory to add the computed values.
  CandidatePairs* Pairs;
  
  // This is the image from which to take the patches
  FloatVectorImageType::Pointer Image;

  // This is the mask to check the validity of target pixels
  Mask::Pointer MaskImage;

  unsigned int NumberOfComponentsPerPixel;
    
  // This function takes a template paramter of a class which has a Difference(pixel, pixel) function.
  template<typename TDifferenceFunction>
  float PatchAverageSourceDifference(const Patch& sourcePatch);

};

#include "SelfPatchCompare.hxx"


#endif
