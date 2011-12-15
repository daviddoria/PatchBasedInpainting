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

#include "PriorityManual.h"

#include "Helpers.h"

PriorityManual::PriorityManual(const FloatVectorImageType* image, const Mask* maskImage, unsigned int patchRadius) : PriorityOnionPeel(image, maskImage, patchRadius)
{
  this->ManualPriorityImage = UnsignedCharScalarImageType::New();
}

float PriorityManual::ComputePriority(const itk::Index<2>& queryPixel)
{
  //std::cout << static_cast<float>(this->ManualPriorityImage->GetPixel(queryPixel)) << std::endl;

  float priority = 0.0f;
  float manualPriority = this->ManualPriorityImage->GetPixel(queryPixel);

  float offset = 1e4;
  if(manualPriority > 0)
    {
    priority = offset + PriorityOnionPeel::ComputePriority(queryPixel);
    }
  else
    {
    priority = PriorityOnionPeel::ComputePriority(queryPixel);
    }

  return priority;
}

void PriorityManual::SetManualPriorityImage(UnsignedCharScalarImageType::Pointer image)
{
  //this->ManualPriorityImage = image;
  Helpers::DeepCopy<UnsignedCharScalarImageType>(image, this->ManualPriorityImage);
}
