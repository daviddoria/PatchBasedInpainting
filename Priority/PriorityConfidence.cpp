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

#include "PriorityConfidence.h"

PriorityConfidence::PriorityConfidence(const Mask* const maskImage, const unsigned int patchRadius) :
MaskImage(maskImage), PatchRadius(patchRadius)
{
  this->ConfidenceMapImage = ITKHelpers::FloatScalarImageType::New();
  InitializeConfidenceMap();
}

void PriorityConfidence::InitializeConfidenceMap()
{
  this->ConfidenceMapImage->SetRegions(this->MaskImage->GetLargestPossibleRegion());
  this->ConfidenceMapImage->Allocate();

  itk::ImageRegionIterator<ConfidenceImageType> imageIterator(this->ConfidenceMapImage,
                                                              this->ConfidenceMapImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(this->MaskImage->IsHole(imageIterator.GetIndex()))
      {
      imageIterator.Set(0.0f);
      }
    else if(this->MaskImage->IsValid(imageIterator.GetIndex()))
      {
      imageIterator.Set(1.0f);
      }

    ++imageIterator;
    }

//   ITKHelpers::WriteImage(ConfidenceMapImage.GetPointer(), "ConfidenceMapInitial.mha");
//   ITKHelpers::WriteScaledScalarImage(ConfidenceMapImage.GetPointer(), "ConfidenceMapInitial.png");
}
