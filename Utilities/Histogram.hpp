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
  std::cout << "Create histogram with " << numberOfBins << " bins." << std::endl;
  std::vector<float> bins(numberOfBins);

  const float binWidth = (rangeMax - rangeMin) / static_cast<float>(numberOfBins);

  std::cout << "binWidth " << binWidth << std::endl;
  
  for(unsigned int i = 0; i < values.size(); ++i)
  {
    int bin = (values[i] - rangeMin) / binWidth;
    if(bin < 0)
    {
      std::stringstream ss;
      ss << "Can't write to bin " << bin << "!";
      throw std::runtime_error(ss.str());
    }
    bins[bin]++;
  }

  return bins;
}

} // end namespace

#endif
