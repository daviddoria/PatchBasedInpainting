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
#include "Helpers.h"
#include "Mask.h"
#include "Patch.h"
#include "Types.h"

// ITK
#include "itkImageRegion.h"

// STL
#include <vector>

class SelfPatchCompare
{
  
public:
  SelfPatchCompare(const unsigned int);
  
  // This function returns the Id of the best source patch, as well as returns the minDistance by reference
  unsigned int FindBestPatch(float& minDistance);

  void SetImage(const FloatVectorImageType::Pointer);

  void SetMask(const Mask::Pointer mask);

  void SetTargetPatch(const Patch&);

  void SetSourcePatches(const std::vector<Patch>&);

  float SlowDifference(const Patch& sourcePatch);
  float PatchDifferenceManual(const Patch& sourcePatch);
  float PatchDifferenceExternal(const Patch& sourcePatch);
  float PatchDifferenceBoundary(const Patch& sourcePatch);

  // Prepare to do some comparisons by finding all of the valid pixels in the target region
  void ComputeOffsets();

  virtual float PixelDifference(const VectorType &a, const VectorType &b) = 0;
  float NonVirtualPixelDifference(const VectorType &a, const VectorType &b);
  
protected:
  // If a channel of one pixel was white (255) and the corresponding channel of the other pixel
  // was black (0), the difference would be 255, so the difference squared would be 255*255
  static const float MaxColorDifference = 255*255;
  
  // These are the offsets of the target region which we with to compare
  std::vector<FloatVectorImageType::OffsetValueType> ValidOffsets;

  // This is the target region we wish to compare. It may be partially invalid.
  Patch TargetPatch;

  // These are the fully valid source regions
  std::vector<Patch> SourcePatches;
  
  // This is the image from which to take the patches
  FloatVectorImageType::Pointer Image;

  // This is the mask to check the validity of target pixels
  Mask::Pointer MaskImage;

  unsigned int NumberOfComponentsPerPixel;

};

#endif
