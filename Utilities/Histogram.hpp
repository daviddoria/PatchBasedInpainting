#ifndef Histogram_HPP
#define Histogram_HPP

#include "Histogram.h"

namespace Histogram
{
  
template <typename TValue>
std::vector<float> ScalarHistogram(const std::vector<TValue>& values, const unsigned int numberOfBins, const TValue& rangeMin, const TValue& rangeMax)
{
  // Count how many values fall in each bin. We store these counts as floats because sometimes we want to normalize the counts.
  std::vector<float> bins(numberOfBins);

  const float binWidth = (rangeMax - rangeMin) / static_cast<float>(numberOfBins);

  for(unsigned int i = 0; i < values.size(); ++i)
  {
    unsigned int bin = (values[i] - rangeMin) / binWidth;
    bins[bin]++;
  }

  return bins;
}

} // end namespace

#endif
