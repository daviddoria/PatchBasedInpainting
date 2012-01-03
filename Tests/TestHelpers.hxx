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

#include "itkImageRegionConstIterator.h"

namespace TestHelpers
{

template<typename TImage>
bool ImagesEqual(const TImage* const image1, const TImage* const image2)
{
  if(image1->GetLargestPossibleRegion() != image2->GetLargestPossibleRegion())
    {
    return false;
    }
  itk::ImageRegionConstIterator<TImage> iterator1(image1, image1->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<TImage> iterator2(image2, image2->GetLargestPossibleRegion());
  while(!iterator1.IsAtEnd())
    {
    // We should not compute derivatives for pixels in the hole.
    if(iterator1.Get() != iterator2.Get())
      {
      return false;
      }
    ++iterator1;
    ++iterator2;
    }
  return true;
}

}// end namespace
