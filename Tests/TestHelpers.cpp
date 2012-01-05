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

#include "TestHelpers.h"

#include <cmath>

namespace TestHelpers
{

bool ValuesEqual(const float a, const float b)
{
  if(fabs(a-b) < 1e-6)
    {
    return true;
    }
  return false;
}

void GetMask(Mask* const mask)
{
  // This function produces a fully valid mask.
  itk::Index<2> corner;
  corner.Fill(0);

  itk::Size<2> size;
  size.Fill(100);

  itk::ImageRegion<2> region(corner,size);
  mask->SetRegions(region);
  mask->Allocate();
  mask->FillBuffer(mask->GetValidValue());
}

} // end namespace
