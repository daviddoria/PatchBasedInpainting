#ifndef NormPixelDifference_hpp
#define NormPixelDifference_hpp

// STL
#include <stdexcept>

/**
 */
template <typename PixelType>
struct NormPixelDifference
{
  float operator()(const PixelType& a, const PixelType& b) const
  {
    return (a-b).GetNorm();
  }
};

#endif
