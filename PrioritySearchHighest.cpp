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

#include "PrioritySearchHighest.h"

// Custom
#include "Helpers/ITKHelpers.h"

// STL
#include <algorithm> // max_element

PrioritySearchHighest::PrioritySearchHighest()
{

}

void PrioritySearchHighest::ComputePriorities(const PixelCollection& boundaryPixels, const Priority* const priority)
{
  for(PixelCollection::ConstIterator pixelIterator = boundaryPixels.begin(); pixelIterator != boundaryPixels.end(); ++pixelIterator)
    {
    PriorityMapType::iterator iter = this->PriorityMap.find(*pixelIterator);
    float priorityValue = 0.0f;
    if(iter == this->PriorityMap.end()) // If the pixel's priority value isn't already in the map, compute it.
      {
      //std::cout << "Not found." << std::endl;
      priorityValue = priority->ComputePriority(*pixelIterator);
      }
    else // If the pixel's priority value is already in the map, return it.
      {
      priorityValue = iter->second;
      }

    itk::Index<2> currentPixel = *pixelIterator;
    this->PriorityMap[currentPixel] = priorityValue;
    }
}

itk::Index<2> PrioritySearchHighest::FindHighestPriority(const PixelCollection& boundaryPixels, const Priority* const priority)
{
  ComputePriorities(boundaryPixels, priority);

  PriorityMapType::iterator iterator = std::max_element(this->PriorityMap.begin(), this->PriorityMap.end(), ValueCompareFunctor());

  return iterator->first;
}

/*
template <typename TImage>
void Priority<TImage>::ComputeAllPriorities()
{
  EnterFunction("ComputeAllPriorities()");

  if(this->MaskImage->GetLargestPossibleRegion() != this->PriorityImage->GetLargestPossibleRegion())
    {
    throw std::runtime_error("Priority::ComputeAllPriorities: The priority image has not been properly initialized!");
    }

  this->MaskImage->FindBoundary(this->BoundaryImage);

  ITKHelpers::SetImageToConstant<FloatScalarImageType>(this->PriorityImage, 0.0f);

  std::vector<itk::Index<2> > boundaryPixels = ITKHelpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage);
  //std::cout << "There are " << boundaryPixels.size() << " boundaryPixels." << std::endl;
  for(unsigned int pixelId = 0; pixelId < boundaryPixels.size(); ++pixelId)
    {
    this->PriorityImage->SetPixel(boundaryPixels[pixelId], ComputePriority(boundaryPixels[pixelId]));
    }

  //std::cout << "Priority image size: " << this->PriorityImage->GetLargestPossibleRegion() << std::endl;

  //HelpersOutput::WriteImage<FloatScalarImageType>(this->PriorityImage, "Debug/Priority.mha");

  LeaveFunction("ComputeAllPriorities()");
}*/
