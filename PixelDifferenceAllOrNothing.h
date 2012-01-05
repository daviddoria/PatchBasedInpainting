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

#ifndef PixelDifferenceAllOrNothing_H
#define PixelDifferenceAllOrNothing_H

#include "PixelDifference.h"

template <typename TScalar>
struct PixelDifferenceAllOrNothing
{
  // Using this class will count how many pixels are not identically the same.
  // This should clearly only be used when comparing int/char-valued images (like a membership image).
  static float Difference(const TScalar& a, const TScalar& b)
  {
    if(a == b) // If the pixels are the same, the difference is 0.
      {
      return 0.0f;
      }
    else // If the pixels are different, the difference is set to 1.
      {
      return 1.0f;
      }

  }
};

#endif
