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

#include "PatchPair.h" // Appease the syntax parser

template <typename TImage>
PatchPair<TImage>::PatchPair(const ImagePatchPixelDescriptor<TImage>* const sourcePatch, const ImagePatchPixelDescriptor<TImage>& targetPatch) :
SourcePatch(sourcePatch), TargetPatch(targetPatch)
{

}

// PatchPair& PatchPair::operator= (const PatchPair& other)
// {
//   if (this != &other)
//   {
//     this->SourcePatch = other.SourcePatch;
//   }
// 
//   return *this;
// }

template <typename TImage>
PatchPairDifferences& PatchPair<TImage>::GetDifferences()
{
  return Differences;
}

template <typename TImage>
const PatchPairDifferences& PatchPair<TImage>::GetDifferences() const
{
  return Differences;
}

template <typename TImage>
itk::Offset<2> PatchPair<TImage>::GetTargetToSourceOffset() const
{
  return this->SourcePatch->GetRegion().GetIndex() - this->TargetPatch.GetRegion().GetIndex();
}

template <typename TImage>
itk::Offset<2> PatchPair<TImage>::GetSourceToTargetOffset() const
{
  return this->TargetPatch.GetRegion().GetIndex() - this->SourcePatch->GetRegion().GetIndex();
}

template <typename TImage>
const ImagePatchPixelDescriptor<TImage>* PatchPair<TImage>::GetSourcePatch() const
{
  return this->SourcePatch;
}

template <typename TImage>
const ImagePatchPixelDescriptor<TImage>& PatchPair<TImage>::GetTargetPatch() const
{
  return this->TargetPatch;
}
/*
float PatchPair::GetDepthAndColorDifference() const
{
  DifferenceMapType::const_iterator colorIter = this->DifferenceMap.find(ColorDifference);

  if(colorIter == this->DifferenceMap.end())
    {
    throw std::runtime_error("Could not compute GetDepthAndColorDifference, ColorDifference not found.");
    }

  DifferenceMapType::const_iterator depthIter = this->DifferenceMap.find(DepthDifference);

  if(depthIter == this->DifferenceMap.end())
    {
    throw std::runtime_error("Could not compute GetDepthAndColorDifference, DepthDifference not found.");
    }

  return ComputeDepthAndColorDifference(this->DifferenceMap.find(DepthDifference)->second, this->DifferenceMap.find(ColorDifference)->second);
}*/

// template <typename TImage>
// bool PairSortFunctor<TImage>::operator()(const PatchPair<TImage>& pair1, const PatchPair<TImage>& pair2)
// {
// //   if(SortOrder == ASCENDING)
// //     {
// //     return (pair1.GetDifferences().GetDifferenceByType(DifferenceType) < pair2.GetDifferences().GetDifferenceByType(DifferenceType));
// //     }
// //   else
// //     {
// //     return !(pair1.GetDifferences().GetDifferenceByType(DifferenceType) < pair2.GetDifferences().GetDifferenceByType(DifferenceType));
// //     }
//   return !(pair1.GetDifferences().GetDifferenceByType(SortBy) < pair2.GetDifferences().GetDifferenceByType(SortBy));
// }

// template <typename TImage>
// bool PairSortFunctor<TImage>::operator()(const std::shared_ptr<PatchPair<TImage> >& pair1, const std::shared_ptr<PatchPair<TImage> >& pair2)
// {
//   return !(pair1->GetDifferences().GetDifferenceByType(SortBy) < pair2->GetDifferences().GetDifferenceByType(SortBy));
// }

template <typename TImage>
void PatchPair<TImage>::VisitAllPixels(PixelPairVisitor<TImage> &visitor) const
{
  itk::ImageRegionConstIteratorWithIndex<TImage> sourceRegionIterator(this->TargetPatch.GetImage(), this->GetSourcePatch()->GetRegion());

  while(!sourceRegionIterator.IsAtEnd())
    {
    visitor.Visit(sourceRegionIterator.Get(), this->TargetPatch.GetImage()->GetPixel(sourceRegionIterator.GetIndex() + this->GetSourceToTargetOffset()));
    ++sourceRegionIterator;
    }
}

template <typename TImage>
void PatchPair<TImage>::VisitAllValidPixels(const Mask* const mask, PixelPairVisitor<TImage> &visitor) const
{
  itk::ImageRegionConstIteratorWithIndex<TImage> targetRegionIterator(this->TargetPatch.GetImage(), this->GetTargetPatch().GetRegion());

  while(!targetRegionIterator.IsAtEnd())
    {
    if(mask->IsValid(targetRegionIterator.GetIndex()))
      {
      visitor.Visit(this->TargetPatch.GetImage()->GetPixel(targetRegionIterator.GetIndex() + this->GetTargetToSourceOffset()), targetRegionIterator.Get());
      }
    ++targetRegionIterator;
    }
}

template <typename TImage>
void PatchPair<TImage>::VisitOffsets(const std::vector<itk::Offset<2> >& offsets, PixelPairVisitor<TImage> &visitor) const
{
  for(unsigned int offsetId = 0; offsetId < offsets.size(); ++offsetId)
    {
    visitor.Visit(this->TargetPatch.GetImage()->GetPixel(this->SourcePatch->GetCorner() + offsets[offsetId]), this->TargetPatch.GetImage()->GetPixel(this->TargetPatch.GetCorner() + offsets[offsetId]));
    }
}
