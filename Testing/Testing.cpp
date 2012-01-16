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

#include "Testing.h"

#include <cmath>

#include "itkImageRegionIteratorWithIndex.h"

namespace Testing
{

bool ValuesEqual(const float a, const float b, const float epsilon)
{
  if(fabs(a-b) < epsilon)
    {
    return true;
    }
  return false;
}

void GetFullyValidMask(Mask* const mask)
{
  // This function produces a fully valid mask.
  itk::ImageRegion<2> region = GetImageRegion();
  mask->SetRegions(region);
  mask->Allocate();
  mask->FillBuffer(mask->GetValidValue());
}

itk::ImageRegion<2> GetImageRegion()
{
  itk::Index<2> corner;
  corner.Fill(0);

  itk::Size<2> size;
  size.Fill(TestImageSize);

  itk::ImageRegion<2> region(corner,size);
  return region;
}

void GetHalfValidMask(Mask* const mask)
{
  // This function produces a fully valid mask.
  itk::ImageRegion<2> region = GetImageRegion();
  mask->SetRegions(region);
  mask->Allocate();
  itk::ImageRegionIteratorWithIndex<Mask> iterator(mask, mask->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    if(static_cast<unsigned int>(iterator.GetIndex()[0]) < TestImageSize/2)
      {
      iterator.Set(mask->GetValidValue());
      }
    else
      {
      iterator.Set(mask->GetHoleValue());
      }
    ++iterator;
    }

}

} // end namespace
