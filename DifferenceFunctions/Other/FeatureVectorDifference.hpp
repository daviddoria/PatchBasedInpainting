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

#ifndef FeatureVectorDifference_hpp
#define FeatureVectorDifference_hpp

// STL
#include <stdexcept>

#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"

/** This functor computes the sum of absolute differences between
  * FeatureVectorPixelDescriptors.
  */
struct FeatureVectorDifference
{
  float operator()(const FeatureVectorPixelDescriptor& a, const FeatureVectorPixelDescriptor& b) const
  {
    // If we are comparing a patch to itself, return infinity.
    // Otherwise, the best match would always be the same patch!
    if(a.GetVertex() == b.GetVertex())
    {
      return std::numeric_limits<float>::infinity();
    }

    // If either patch is invalid, the comparison cannot be performed.
    if(a.GetStatus() == PixelDescriptor::INVALID || b.GetStatus() == PixelDescriptor::INVALID)
    {
      return std::numeric_limits<float>::infinity();
    }

    float totalDifference = 0.0f;

    assert(a.GetFeatureVector().size() == b.GetFeatureVector().size());

    for(unsigned int i = 0; i < a.GetFeatureVector().size(); ++i)
    {
      totalDifference += fabs(a.GetFeatureVector()[i] - b.GetFeatureVector()[i]);
    }

    //std::cout << "totalDifference: " << totalDifference << std::endl;
    return totalDifference;
  }
};

#endif
