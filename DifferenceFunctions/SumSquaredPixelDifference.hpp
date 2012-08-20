#ifndef SumSquaredPixelDifference_hpp
#define SumSquaredPixelDifference_hpp

// STL
#include <stdexcept>

// Custom
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKContainerInterface.h"

/**
 */
template <typename PixelType>
struct SumSquaredPixelDifference
{
  float operator()(const PixelType& a, const PixelType& b) const
  {
    assert(Helpers::length(a) == Helpers::length(b));

    float pixelDifference = 0.0f;
    for(unsigned int component = 0; component < Helpers::length(a); ++component)
      {
      float componentDifference = (Helpers::index(a,component) - Helpers::index(b,component)) *
                                  (Helpers::index(a,component) - Helpers::index(b,component));
      pixelDifference += componentDifference;
      }
    return pixelDifference;
  }
};

#endif
