#ifndef SumAbsolutePixelDifferenceN_hpp
#define SumAbsolutePixelDifferenceN_hpp

// STL
#include <stdexcept>

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

/**
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
