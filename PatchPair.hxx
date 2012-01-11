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
void PatchPair::VisitAllPixels(const TImage* const image, PixelPairVisitor<TImage> &visitor) const
{
  itk::ImageRegionConstIteratorWithIndex<TImage> sourceRegionIterator(image, this->GetSourcePatch()->GetRegion());

  while(!sourceRegionIterator.IsAtEnd())
    {
    visitor.Visit(sourceRegionIterator.Get(), image->GetPixel(sourceRegionIterator.GetIndex() + this->GetSourceToTargetOffset()));
    ++sourceRegionIterator;
    }
}

template <typename TImage>
void PatchPair::VisitAllValidPixels(const TImage* const image, const Mask* const mask, PixelPairVisitor<TImage> &visitor) const
{
  itk::ImageRegionConstIteratorWithIndex<TImage> targetRegionIterator(image, this->GetTargetPatch().GetRegion());

  while(!targetRegionIterator.IsAtEnd())
    {
    if(mask->IsValid(targetRegionIterator.GetIndex()))
      {
      visitor.Visit(image->GetPixel(targetRegionIterator.GetIndex() + this->GetTargetToSourceOffset()), targetRegionIterator.Get());
      }
    ++targetRegionIterator;
    }
}

template <typename TImage>
void PatchPair::VisitOffsets(const TImage* const image, const std::vector<itk::Offset<2> >& offsets, PixelPairVisitor<TImage> &visitor) const
{
  for(unsigned int offsetId = 0; offsetId < offsets.size(); ++offsetId)
    {
    visitor.Visit(image->GetPixel(this->SourcePatch->GetCorner() + offsets[offsetId]), image->GetPixel(this->TargetPatch.GetCorner() + offsets[offsetId]));
    }
}
