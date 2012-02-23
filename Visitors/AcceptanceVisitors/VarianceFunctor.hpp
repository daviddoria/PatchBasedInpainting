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
  float operator()(const std::vector<TPixel>& pixels) const
  {
    return ITKHelpers::SumOfComponents(Statistics::Variance(pixels));
  }
};

#endif
