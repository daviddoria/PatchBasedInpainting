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

#include "PriorityManual.h" // Appease syntax parser

#include "Helpers/ITKHelpers.h"

template< typename TImage, typename TPriority>
PriorityManual<TImage, TPriority>::PriorityManual(const TImage* manualPriorityImage, Priority* const priorityFunction)
{
  this->ManualPriorityImage = UnsignedCharScalarImageType::New();
}

// template< typename TImage, typename TPriority>
// void SetPriorityFunction(Priority* const priorityFunction)
// {
//   this->PriorityFunction = priorityFunction;
// }

template< typename TImage, typename TPriority>
float PriorityManual<TImage, TPriority>::ComputePriority(const itk::Index<2>& queryPixel) const
{
  //std::cout << static_cast<float>(this->ManualPriorityImage->GetPixel(queryPixel)) << std::endl;

  float priority = 0.0f;
  float manualPriority = this->ManualPriorityImage->GetPixel(queryPixel);

  // Make the priority values extremely high, but still sorted.
  float offset = 1e4;
  float normalPriority = this->PriorityFunction->ComputePriority(queryPixel);
  if(manualPriority > 0)
    {
    priority = offset + normalPriority;
    }
  else
    {
    priority = normalPriority;
    }

  return priority;
}

// template< typename TImage, typename TPriority>
// UnsignedCharScalarImageType* PriorityManual<TImage, TPriority>::GetManualPriorityImage()
// {
//   return this->ManualPriorityImage;
// }

template< typename TImage, typename TPriority>
void PriorityManual<TImage, TPriority>::Update(const itk::Index<2>& filledPixel)
{

}

template< typename TImage, typename TPriority>
void PriorityManual<TImage, TPriority>::SetManualPriorityImage(UnsignedCharScalarImageType* const image)
{
  //this->ManualPriorityImage = image;
  ITKHelpers::DeepCopy<UnsignedCharScalarImageType>(image, this->ManualPriorityImage);
}
