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
#include "Types.h"

// ITK
#include "itkImageRegion.h"

// STL
#include <vector>

template< typename TImage>
class SelfPatchCompare
{
public:
  float PatchDifference(const itk::ImageRegion<2> sourceRegion);

  float PatchDifference(const TImage* image, const Mask* mask, const itk::Index<2> queryPixel, const itk::Index<2> currentPixel, const unsigned int patchRadius);

  unsigned int FindBestPatch();

  void SetImage(typename TImage::Pointer);

  void SetMask(Mask::Pointer mask);

  void SetTargetRegion(const itk::ImageRegion<2>);

  void SetSourceRegions(const std::vector<itk::ImageRegion<2> >&);
  
private:
  // These are the offsets of the target region which we with to compare
  std::vector<typename TImage::OffsetValueType> ValidOffsets;

  void ComputeOffsets();

  // This is the target region we wish to compare. It may be partially invalid.
  itk::ImageRegion<2> TargetRegion;

  // These are the fully valid source regions
  std::vector<itk::ImageRegion<2> > SourceRegions;
  
  // This is the image from which to take the patches
  typename TImage::Pointer Image;

  // This is the mask to check the validity of target pixels
  Mask::Pointer MaskImage;
};

#include "SelfPatchCompare.hxx"

#endif