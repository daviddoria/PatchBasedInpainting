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

    return angle;
  }
};

#endif
