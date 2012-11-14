#ifndef Histogram_H
#define Histogram_H

// template <int TDimension> // Don't want to template on the dimension because pixel dimension is unknown at compile time.
namespace Histogram
{
  template <typename TValue>
  std::vector<float> ScalarHistogram(const std::vector<TValue>& values, const unsigned int numberOfBins,
                                     const TValue& rangeMin, const TValue& rangeMax);
                                     //const TValue& rangeMin = 0, const TValue& rangeMax = 255);
};

#include "Histogram.hpp"

#endif
