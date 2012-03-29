#ifndef AverageFunctor_HPP
#define AverageFunctor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "ITKHelpers/ITKHelpers.h"
#include "Helpers/Statistics.h"

/**

 */
struct AverageFunctor
{
  template <typename TPixel>
  typename TypeTraits<TPixel>::LargerType operator()(const std::vector<TPixel>& pixels) const
  {
    typename TypeTraits<TPixel>::LargerType allChannelsAverage = Statistics::Average(pixels);
    // std::cout << "AverageFunctor() : allChannelsAverage " << allChannelsAverage << std::endl;

    return allChannelsAverage;
  }
};

#endif
