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

#include "SourcePatchCollection.h" // Appease syntax parser

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"

template <typename TImage>
SourcePatchCollection<TImage>::SourcePatchCollection(const TImage* const image, const Mask* const maskImage, const unsigned int patchRadius) : Image(image), MaskImage(maskImage), PatchRadius(patchRadius)
{

}

template <typename TImage>
void SourcePatchCollection<TImage>::Clear()
{
  this->SourcePatches.clear();
}

template <typename TImage>
typename SourcePatchCollection<TImage>::Iterator SourcePatchCollection<TImage>::begin() const
{
  return SourcePatches.begin();
}

template <typename TImage>
typename SourcePatchCollection<TImage>::Iterator SourcePatchCollection<TImage>::end() const
{
  return SourcePatches.end();
}

template <typename TImage>
const ImagePatchPixelDescriptor<TImage>* SourcePatchCollection<TImage>::GetPatch(const itk::ImageRegion<2>& region)
{
  for(Iterator iterator = this->SourcePatches.begin(); iterator != this->SourcePatches.end(); ++iterator)
    {
    if((*iterator).GetRegion() == region)
      {
      return &(*iterator);
      }
    }
  assert("Requested a patch that is not in the collection!" && 0); // Should never get here.
  return NULL;
}

template <typename TImage>
typename SourcePatchCollection<TImage>::PatchContainer SourcePatchCollection<TImage>::FindSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid to the list of source patches.

  PatchContainer patches;
  // Clearly we cannot add source patches from regions that are outside the image, so crop the desired region to be inside the image.
  itk::ImageRegion<2> newRegion = ITKHelpers::CropToRegion(region, this->MaskImage->GetLargestPossibleRegion());

  itk::ImageRegionConstIterator<Mask> iterator(this->MaskImage, newRegion);

  while(!iterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = iterator.GetIndex();
    itk::ImageRegion<2> currentPatchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius);

    if(this->MaskImage->GetLargestPossibleRegion().IsInside(currentPatchRegion))
      {
      if(this->MaskImage->IsValid(currentPatchRegion))
        {
        patches.insert(ImagePatchPixelDescriptor<TImage>(this->Image, currentPatchRegion));
        }
      }

    ++iterator;
    }
  return patches;
}

template <typename TImage>
void SourcePatchCollection<TImage>::AddPatches(const SourcePatchCollection<TImage>::PatchContainer& patches)
{
  std::set_union(this->SourcePatches.begin(), this->SourcePatches.end(), patches.begin(), patches.end(),
                 std::inserter<PatchContainer>(this->SourcePatches, this->SourcePatches.end()));
}

template <typename TImage>
unsigned int SourcePatchCollection<TImage>::GetNumberOfPatches() const
{
  return this->SourcePatches.size();
}
