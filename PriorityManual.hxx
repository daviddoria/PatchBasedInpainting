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

#include "Helpers/ITKHelpers.h"

template< typename TImage, template<class> class TPriority>
PriorityManual<TImage, TPriority>::PriorityManual(const TImage* image, const Mask* maskImage, unsigned int patchRadius) :
TPriority<TImage>(image, maskImage, patchRadius)
{
  this->ManualPriorityImage = UnsignedCharScalarImageType::New();
}

template< typename TImage, template<class> class TPriority>
float PriorityManual<TImage, TPriority>::ComputePriority(const itk::Index<2>& queryPixel)
{
  //std::cout << static_cast<float>(this->ManualPriorityImage->GetPixel(queryPixel)) << std::endl;

  float priority = 0.0f;
  float manualPriority = this->ManualPriorityImage->GetPixel(queryPixel);

  float offset = 1e4;
  if(manualPriority > 0)
    {
    priority = offset + PriorityOnionPeel<TImage>::ComputePriority(queryPixel);
    }
  else
    {
    priority = PriorityOnionPeel<TImage>::ComputePriority(queryPixel);
    }

  return priority;
}

template< typename TImage, template<class> class TPriority>
UnsignedCharScalarImageType* PriorityManual<TImage, TPriority>::GetManualPriorityImage()
{
  return this->ManualPriorityImage;
}

template< typename TImage, template<class> class TPriority>
void PriorityManual<TImage, TPriority>::SetManualPriorityImage(UnsignedCharScalarImageType* const image)
{
  //this->ManualPriorityImage = image;
  ITKHelpers::DeepCopy<UnsignedCharScalarImageType>(image, this->ManualPriorityImage);
}
