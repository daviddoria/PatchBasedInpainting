#ifndef Histogram_HPP
#define Histogram_HPP

#include "Histogram.h"

#include <iostream>
#include <stdexcept>

namespace Histogram
{
  
template <typename TValue>
std::vector<float> ScalarHistogram(const std::vector<TValue>& values, const unsigned int numberOfBins,
                                   const TValue& rangeMin, const TValue& rangeMax)
{
  // Count how many values fall in each bin. We store these counts as floats because sometimes we want to normalize the counts.
  // std::cout << "Create histogram with " << numberOfBins << " bins." << std::endl;
  std::vector<float> bins(numberOfBins, 0);

  const float binWidth = (rangeMax - rangeMin) / static_cast<float>(numberOfBins);

  if(fabs(binWidth - 0.0f) < 1e-6)
  {
    return bins;
  }
  
  for(unsigned int i = 0; i < values.size(); ++i)
  {
    int bin = (values[i] - rangeMin) / binWidth;
    if(bin < 0)
    {
      std::stringstream ss;
      ss << "Can't write to bin " << bin << "!" << std::endl;
      ss << "There are " << values.size() << " values." << std::endl;
      ss << "Range min " << rangeMin << std::endl;
      ss << "Range max " << rangeMax << std::endl;
      ss << "values[i] (i = " << i << ") = " << values[i] << std::endl;
      ss << "binWidth " << binWidth << std::endl;
      throw std::runtime_error(ss.str());
    }
    bins[bin]++;
  }

  return bins;
}

} // end namespace

#endif
