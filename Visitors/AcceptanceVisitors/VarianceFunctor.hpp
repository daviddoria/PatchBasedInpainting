#ifndef VarianceFunctor_HPP
#define VarianceFunctor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "Utilities/Statistics.h"
#include "Helpers/ITKHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
struct VarianceFunctor
{
  template <typename TPixel>
  typename TypeTraits<TPixel>::LargerType operator()(const std::vector<TPixel>& pixels) const
  {
    assert(pixels.size() > 0);
//     if(pixels.size() <= 0)
//     {
//       throw std::runtime_error("Must have more than 0 items to use VarianceFunctor!");
//     }
    typename TypeTraits<TPixel>::LargerType allChannelsVariance = Statistics::Variance(pixels);
    return allChannelsVariance;
  }
};

#endif
