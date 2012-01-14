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

#include "ValidRegionIterator.h"

#include "Helpers/ITKHelpers.h"

ValidRegionIterator::ValidRegionIterator(const Mask* const mask, const itk::ImageRegion<2>& region, const unsigned int patchRadius):
MaskImage(mask), PatchRadius(patchRadius)
{
  itk::ImageRegionConstIterator<Mask> maskIterator(mask, region);

  // Create all of the valid regions and store them in a vector
  while(!maskIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = maskIterator.GetIndex();
    itk::ImageRegion<2> currentPatchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius);

    if(this->MaskImage->GetLargestPossibleRegion().IsInside(currentPatchRegion))
      {
      if(this->MaskImage->IsValid(currentPatchRegion))
        {
        this->ValidRegions.push_back(currentPatchRegion);
        }
      }

    ++maskIterator;
    }
}


ValidRegionIterator::ConstIterator ValidRegionIterator::begin() const
{
  return this->ValidRegions.begin();
}

ValidRegionIterator::ConstIterator ValidRegionIterator::end() const
{
  return this->ValidRegions.end();
}
