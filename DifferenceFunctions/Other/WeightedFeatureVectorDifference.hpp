#ifndef WeightedFeatureVectorDifference_hpp
#define WeightedFeatureVectorDifference_hpp

// STL
#include <stdexcept>

#include "PixelDescriptors/FeatureVectorPixelDescriptor.h"

struct WeightedFeatureVectorDifference
{
  std::vector<float> Weights;
  
  float operator()(const FeatureVectorPixelDescriptor& a, const FeatureVectorPixelDescriptor& b) const
  {
    assert(Weights.size() == a.GetFeatureVector().size());

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

    float totalDifference = 0.0f;

    assert(a.GetFeatureVector().size() == b.GetFeatureVector().size());
    assert(a.GetFeatureVector().size() == Weights.size());

    for(unsigned int i = 0; i < a.GetFeatureVector().size(); ++i)
      {
      totalDifference += Weights[i] * fabs(a.GetFeatureVector()[i] - b.GetFeatureVector()[i]);
      }

    //std::cout << "totalDifference: " << totalDifference << std::endl;
    return totalDifference;
  }
};

#endif
