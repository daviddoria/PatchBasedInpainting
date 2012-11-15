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

#ifndef SumAbsolutePixelDifference_hpp
#define SumAbsolutePixelDifference_hpp

// STL
#include <stdexcept>

// Custom
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKContainerInterface.h"

/**
  * This functor computes the sum of absolute differences of the components of ND pixels.
  */
template <typename PixelType>
struct SumAbsolutePixelDifference
{
  float operator()(const PixelType& a, const PixelType& b) const
  {
    assert(Helpers::length(a) == Helpers::length(b));

    float pixelDifference = 0.0f;
    for(unsigned int component = 0; component < Helpers::length(a); ++component)
    {
      float componentDifference = fabs(Helpers::index(a,component) - Helpers::index(b,component));
      pixelDifference += componentDifference;
    }
    return pixelDifference;
  }
};

#endif
