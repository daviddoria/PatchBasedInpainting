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

#include "CandidatePairs.h" // Appease syntax parser

#include <stdexcept>

template <typename TImage>
void CandidatePairs<TImage>::VisitAllPatchPairs(const TImage* const image, const Mask* const mask, PatchPairVisitor<TImage> &visitor)
{
//   #ifdef USE_QT_PARALLEL
//     #error Parallel version not yet implemented
//     #pragma message("Using QtConcurrent!")
//     QVector<float> differences = QtConcurrent::blockingMapped<QVector<float> >(this->begin(), this->end(),
//                                                                                boost::bind(&PixelPairVisitor::VisitOffsets, _1, _2, _3));
//   #else
//     #pragma message("NOT using QtConcurrent!")
// 
//     std::vector<itk::Offset<2> > offsets = ComputeOffsets(mask);
// 
//     for(ConstIterator patchIterator = this->begin(); patchIterator != this->end(); ++patchIterator)
//       {
//       (*patchIterator).VisitOffsets(image, offsets, visitor);
//       }
//   #endif
  
  std::vector<itk::Offset<2> > offsets = ComputeOffsets(mask);

  for(ConstIterator patchIterator = this->begin(); patchIterator != this->end(); ++patchIterator)
    {
    //(*patchIterator).VisitOffsets(image, offsets, visitor);
    visitor.Visit(*patchIterator);
    }
}

template <typename TImage>
CandidatePairs<TImage>::CandidatePairs(const ImagePatchItem<TImage>& targetPatch) : Priority(0.0f), TargetPatch(targetPatch)
{

}

template <typename TImage>
std::vector<itk::Offset<2> > CandidatePairs<TImage>::ComputeOffsets(const Mask* const mask)
{
  // This function computes the list of offsets that are from the source region of the target patch.
  std::vector<itk::Offset<2> > offsets;
  
  // Iterate over the target region of the mask. Add the linear offset of valid pixels to the offsets to be used later in the comparison.
  itk::ImageRegionConstIterator<Mask> maskIterator(mask, this->GetTargetPatch().GetRegion());
  itk::Index<2> targetCorner = this->GetTargetPatch().GetCorner();
  while(!maskIterator.IsAtEnd())
    {
    if(mask->IsValid(maskIterator.GetIndex()))
      {
      // The ComputeOffset function returns the linear index of the pixel.
      // To compute the memory address of the pixel, we must multiply by the number of components per pixel.
      itk::Offset<2> offset = maskIterator.GetIndex() - targetCorner;

      //std::cout << "Using offset: " << offset << std::endl;
      offsets.push_back(offset); // We have to multiply the linear offset by the number of components per pixel for the VectorImage type
      }

    ++maskIterator;
    }
  return offsets;
}

template <typename TImage>
void CandidatePairs<TImage>::Sort(const PatchPairDifferences::PatchPairDifferenceTypes sortBy, const SortOrderEnum ordering)
{
//   PairSortFunctor sortFunctor(sortBy);
//   std::sort(this->PatchPairs.begin(), this->PatchPairs.end(), sortFunctor);
}

template <typename TImage>
void CandidatePairs<TImage>::AddSourcePatches(const SourcePatchCollection<TImage>& patches)
{
  for(typename SourcePatchCollection<TImage>::Iterator patchIterator = patches.begin(); patchIterator != patches.end(); ++patchIterator)
    {
    const ImagePatchItem<TImage>* sourcePatch = &(*patchIterator);
    // TODO: Why doesn't this need typename PatchPair<TImage> ? isn't PatchPair<TImage> a dependent type?
    std::shared_ptr<PatchPair<TImage> > patchPair = std::shared_ptr<PatchPair<TImage> >(new PatchPair<TImage>(sourcePatch, this->TargetPatch));
    this->PatchPairs.push_back(patchPair);
    }
}

template <typename TImage>
typename CandidatePairs<TImage>::Iterator CandidatePairs<TImage>::begin()
{
  return this->PatchPairs.begin();
}

template <typename TImage>
typename CandidatePairs<TImage>::Iterator CandidatePairs<TImage>::end()
{
  return this->PatchPairs.end();
}

template <typename TImage>
typename CandidatePairs<TImage>::ConstIterator CandidatePairs<TImage>::begin() const
{
  return this->PatchPairs.begin();
}

template <typename TImage>
typename CandidatePairs<TImage>::ConstIterator CandidatePairs<TImage>::end() const
{
  return this->PatchPairs.end();
}

// const PatchPair& CandidatePairs::GetPair(const unsigned int pairId) const
// {
//   return *(this->PatchPairs[pairId]);
// }
// 
// PatchPair& CandidatePairs::GetPair(const unsigned int pairId)
// {
//   return *(this->PatchPairs[pairId]);
// }

// const Patch* const CandidatePairs::GetSourcePatch(const unsigned int pairId) const
// {
//   return this->PatchPairs[pairId]->GetSourcePatch();
// }

template <typename TImage>
const ImagePatchItem<TImage>& CandidatePairs<TImage>::GetTargetPatch() const
{
  return this->TargetPatch;
}

template <typename TImage>
unsigned int CandidatePairs<TImage>::GetNumberOfSourcePatches() const
{
  return this->PatchPairs.size();
}

// void CandidatePairs::AddCandidatePair(const PatchPair& patchPair)
// {
//   assert(patchPair.GetTargetPatch() == this->PatchPairs[0]->GetTargetPatch());
// 
//   this->PatchPairs.push_back(std::shared_ptr<PatchPair>(new PatchPair(patchPair)));
// }

template <typename TImage>
float CandidatePairs<TImage>::GetPriority() const
{
  return this->Priority;
}

template <typename TImage>
void CandidatePairs<TImage>::SetPriority(const float priority)
{
  this->Priority = priority;
}

// std::vector<PatchPair> CandidatePairs::GetAllPairs() const
// {
//   std::vector<PatchPair> pairs;
//   for(unsigned int patchId = 0; patchId < this->PatchPairs.size(); ++patchId)
//     {
//     pairs.push_back(PatchPair(this->GetTargetPatch(), this->GetSourcePatch(patchId)));
//     }
//   return pairs;
// }

template <typename TImage>
std::vector<std::shared_ptr<PatchPair<TImage> > > CandidatePairs<TImage>::GetPatchPairs()
{
  return this->PatchPairs;
}

template <typename TImage>
void CandidatePairs<TImage>::Combine(const CandidatePairs<TImage>& candidatePairs)
{
  if(candidatePairs.GetTargetPatch() != this->GetTargetPatch())
    {
    throw std::runtime_error("Cannot combine CandidatePairs that are not of the same TargetPatch!");
    }
//   for(unsigned int patchId = 0; patchId < candidatePairs.GetNumberOfSourcePatches(); ++patchId)
//     {
//     this->PatchPairs.push_back(std::shared_ptr<PatchPair>(new PatchPair(candidatePairs.GetPair(patchId))));
//     }
  for(ConstIterator patchIterator = candidatePairs.begin(); patchIterator != candidatePairs.end(); ++patchIterator)
    {
    this->PatchPairs.push_back(std::shared_ptr<PatchPair<TImage> >(new PatchPair<TImage>(*patchIterator)));
    }
}


// void CandidatePairs::OutputPairs(const std::vector<PatchPair>& patchPairs, const std::string& filename)
// {
//   std::ofstream fout(filename.c_str());
// 
//   for(unsigned int i = 0; i < patchPairs.size(); ++i)
//     {
//     fout << "Potential patch " << i << ": " << std::endl
//         << "target index: " << patchPairs[i].TargetPatch.Region.GetIndex() << std::endl;
//         //<< "ssd score: " << patchPairs[i].GetAverageSSD() << std::endl;
//         //<< "histogram score: " << patchPairs[i].HistogramDifference << std::endl;
//     }
// 
//   fout.close();
// }
