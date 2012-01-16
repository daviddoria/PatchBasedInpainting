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

#ifndef CandidatePairs_H
#define CandidatePairs_H

// Custom
#include "PatchPairVisitor.h"
#include "PatchPair.h"
#include "SourcePatchCollection.h"

// Boost
#include <boost/iterator/indirect_iterator.hpp>

// STL
#include <vector>

/**
\class CandidatePairs
\brief This class stores a target patch (a pointer to one of the source patches in PatchBasedInpainting)
       and a list of N (user specified) source patches (also pointers to source patches in PatchBasedInpainting)
*/
template <typename TImage>
class CandidatePairs
{
private:
  /** This is a vector because the pairs are ordered - they can be sorted by any of their Distance
      values.
    */
  typedef std::vector<std::shared_ptr<PatchPair<TImage> > > PatchContainer;

public:
  /** Constructor that creates the CandidatePairs collection for pairs of 'targetPatch'.*/
  CandidatePairs(const ImagePatchItem<TImage>& targetPatch);

  /** Apply a visitor to each PatchPair.*/
  void VisitAllPatchPairs(const TImage* const image, const Mask* const mask, PatchPairVisitor<TImage> &visitor);

  // Iterator interface
  typedef boost::indirect_iterator<typename PatchContainer::iterator> Iterator;
  typedef boost::indirect_iterator<typename PatchContainer::const_iterator> ConstIterator;
  Iterator begin();
  Iterator end();
  ConstIterator begin() const;
  ConstIterator end() const;

  /** Add source patches to the collection.*/
  void AddSourcePatches(const SourcePatchCollection<TImage>& patches);

  /** Absorb the source patches of another CandidatePairs collection.*/
  void Combine(const CandidatePairs<TImage>& pairs);

  /** Get all of the PatchPairs. */
  std::vector<std::shared_ptr<PatchPair<TImage> > > GetPatchPairs();

  /** Determine the order in which the PatchPairs will be sorted. */
  enum SortOrderEnum {ASCENDING, DESCENDING};

  /** Sort the PatchPairs on a particular difference type. */
  void Sort(const PatchPairDifferences::PatchPairDifferenceTypes sortBy, const SortOrderEnum ordering = DESCENDING);

  /** Get the target patch. */
  const ImagePatchItem<TImage>& GetTargetPatch() const;

  /** Get the priority of the target patch, and hence of the collection. */
  float GetPriority() const;

  /** Set the priority of the target patch, and hence of the collection. */
  void SetPriority(const float priority);

  /** Get the number of source patches in the collection. */
  unsigned int GetNumberOfSourcePatches() const;

private:

  // Prepare to do some comparisons by finding all of the valid pixels in the target region
  std::vector<itk::Offset<2> >  ComputeOffsets(const Mask* const mask);

  PatchContainer PatchPairs;

  float Priority;

  const ImagePatchItem<TImage> TargetPatch;
};

#include "CandidatePairs.hxx"

#endif
