#ifndef SumAbsolutePixelDifference_hpp
#define SumAbsolutePixelDifference_hpp

// STL
#include <stdexcept>

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

/**
 */
template <typename PixelType>
struct SumAbsolutePixelDifference
{
  float operator()(const PixelType& a, const PixelType& b) const
  {
    using Helpers::length;
    using ITKHelpers::length;
    using Helpers::index;
    using ITKHelpers::index;
    assert(length(a) == length(b));
    
    float pixelDifference = 0.0f;
    for(unsigned int component = 0; component < ITKHelpers::length(a); ++component)
      {
      float componentDifference = fabs(index(a,component) - index(b,component));
      pixelDifference += componentDifference;
      }
    return pixelDifference;
  }
};

#endif
