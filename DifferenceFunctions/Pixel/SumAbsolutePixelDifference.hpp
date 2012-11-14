#ifndef SumAbsolutePixelDifference_hpp
#define SumAbsolutePixelDifference_hpp

// STL
#include <stdexcept>

// Custom
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKContainerInterface.h"

/**
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
