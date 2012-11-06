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

#ifndef FeatureVectorAngleDifference_hpp
#define FeatureVectorAngleDifference_hpp

// STL
#include <stdexcept>
#include <cmath>

// Custom
#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"

// This functor assumes the feature vectors are already normalized!
struct FeatureVectorAngleDifference
{
  float operator()(const FeatureVectorPixelDescriptor& a, const FeatureVectorPixelDescriptor& b) const
  {
    // If we are comparing a patch to itself, return inf. Otherwise, the best match would always be the same patch!
    if(a.GetVertex() == b.GetVertex())
      {
      return std::numeric_limits<float>::infinity();
      }

    // If either patch is invalid, the comparison cannot be performed.
    if(a.GetStatus() == PixelDescriptor::INVALID || b.GetStatus() == PixelDescriptor::INVALID)
      {
      return std::numeric_limits<float>::infinity();
      }

    float dotProduct = 0.0f;

    assert(a.GetFeatureVector().size() == b.GetFeatureVector().size());

    for(unsigned int i = 0; i < a.GetFeatureVector().size(); ++i)
      {
      dotProduct += a.GetFeatureVector()[i] * b.GetFeatureVector()[i];
      }

    //std::cout << "dotProduct: " << dotProduct << std::endl;
    
    float angle = acos(dotProduct); // This is only correct if this is the dot product of the normalized vectors
    //std::cout << "angle: " << angle << std::endl;
    //std::cout << "a[0]: " << a.GetFeatureVector()[0] << " b[0]: " << b.GetFeatureVector()[0] << std::endl;
//     if(angle > 1.5)
//       {
//       std::cout << "angle: " << angle << " a[0]: " << a.GetFeatureVector()[0] << " b[0]: " << b.GetFeatureVector()[0] << std::endl;
//       }
    
    // Since we don't trust the normals to be correctly oriented, this makes sure the acute angle between vectors is returned.
    // (Simply subtrace 90 degrees if the angle is greater than 90 degrees).
    if(angle > 3.14f / 2.0f)
      {
      angle = angle - (3.14f / 2.0f);
      }
    return angle;
  }
};

#endif
