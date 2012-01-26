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

#ifndef PATCHPAIR_H
#define PATCHPAIR_H

#include "PatchPairDifferences.h"
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "PixelPairVisitor.h"

#include <memory>

/**
\class PatchPair
\brief A source patch and a target patch.
*/
template <typename TImage>
class PatchPair
{
public:

  /** Construct a PatchPair between a source patch and a target patch*/
  PatchPair(const ImagePatchPixelDescriptor<TImage>* const sourcePatch, const ImagePatchPixelDescriptor<TImage>& targetPatch);

  /** Compute the relative location of the source and target patch corners*/
  itk::Offset<2> GetTargetToSourceOffset() const;

  /** Get the relative location from the source patch to the target patch.*/
  itk::Offset<2> GetSourceToTargetOffset() const;

  /** Get the source patch.*/
  const ImagePatchPixelDescriptor<TImage>* GetSourcePatch() const;

  /** Get the target patch.*/
  const ImagePatchPixelDescriptor<TImage>& GetTargetPatch() const;

  /** Get the differences of the PatchPair.*/
  PatchPairDifferences& GetDifferences();

  /** Get the differences of the PatchPair.*/
  const PatchPairDifferences& GetDifferences() const;

  /** Visit all corresponding pairs of pixels.*/
  void VisitAllPixels(PixelPairVisitor<TImage> &visitor) const;

  /** Visit all corresponding pairs of pixels where the mask is valid.*/
  void VisitAllValidPixels(const Mask* const mask, PixelPairVisitor<TImage> &visitor) const;

  /** Visit corresponding pairs of pixels at the specified offsets.*/
  void VisitOffsets(const std::vector<itk::Offset<2> >& offsets, PixelPairVisitor<TImage> &visitor) const;

private:
  PatchPairDifferences Differences;

  const ImagePatchPixelDescriptor<TImage>* const SourcePatch; // This is a pointer to a patch in the main (only) SourcePatchCollection.

  ImagePatchPixelDescriptor<TImage> TargetPatch; // This is not a pointer to a patch in the main SourcePatchCollection, because it is not valid yet (we are going to copy pixels here)!

};

// struct PairSortFunctor
// {
//   PairSortFunctor(const PatchPairDifferences::PatchPairDifferenceTypes sortBy) : SortBy(sortBy) {}
//   bool operator()(const PatchPair<TImage>& pair1, const PatchPair<TImage>& pair2);
// 
//   bool operator()(const std::shared_ptr<PatchPair<TImage> >& pair1, const std::shared_ptr<PatchPair<TImage> >& pair2);
// 
//   PatchPairDifferences::PatchPairDifferenceTypes SortBy;
// };

#include "PatchPair.hxx"

#endif
