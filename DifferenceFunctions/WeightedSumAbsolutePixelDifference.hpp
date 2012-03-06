#ifndef WeightedSumAbsolutePixelDifference_hpp
#define WeightedSumAbsolutePixelDifference_hpp

// STL
#include <stdexcept>

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"

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
