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

#ifndef WeightedSumAbsolutePixelDifference_hpp
#define WeightedSumAbsolutePixelDifference_hpp

// STL
#include <stdexcept>

// Custom
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>

/**
 */
template <typename PixelType>
struct WeightedSumAbsolutePixelDifference
{
  std::vector<float> Weights;
  
  float operator()(const PixelType& a, const PixelType& b) const
  {
    using Helpers::length;
    using ITKHelpers::length;
    using Helpers::index;
    using ITKHelpers::index;
    assert(length(a) == length(b));

    //assert(length(a) == Weights.size());
    if(length(a) != Weights.size())
    {
      std::stringstream ss;
      ss << "length(a) != Weights.size(). a is " << length(a) << " and weights is " << Weights.size();
      throw std::runtime_error(ss.str());
    }
    
    float pixelDifference = 0.0f;
    for(unsigned int component = 0; component < length(a); ++component)
      {
      float componentDifference = Weights[component] * fabs(index(a,component) - index(b,component));
      pixelDifference += componentDifference;
      }
    return pixelDifference;
  }
};

#endif
