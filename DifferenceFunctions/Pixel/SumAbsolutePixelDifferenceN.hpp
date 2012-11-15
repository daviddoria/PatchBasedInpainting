/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef SumAbsolutePixelDifferenceN_hpp
#define SumAbsolutePixelDifferenceN_hpp

// STL
#include <stdexcept>

// Custom
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKHelpers.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

/**
  * This functor computes the sum of absolute differences of the first
  * N dimensions of any length D pixels (where of course D >= N).
  */
template <typename PixelType>
struct SumAbsolutePixelDifferenceN
{
  /** This is the number of components to compare. The number of components of the pixels must be at least N. */
  unsigned int N;
  
  SumAbsolutePixelDifferenceN(const unsigned int n) : N(n) {}
  
  float operator()(const PixelType& a, const PixelType& b) const
  {
    using Helpers::length;
    using ITKHelpers::length;
    using Helpers::index;
    using ITKHelpers::index;
    assert(length(a) == length(b));

    float pixelDifference = 0.0f;
    for(unsigned int component = 0; component < N; ++component)
    {
      float componentDifference = fabs(index(a,component) - index(b,component));
      pixelDifference += componentDifference;
    }
    return pixelDifference;
  }
};

#endif
