#ifndef AverageFunctor_HPP
#define AverageFunctor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/BoostHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
struct AverageFunctor
{
  template <typename TPixel>
  float operator()(const std::vector<TPixel>& pixels)
  {
    return ITKHelpers::SumOfComponents(ITKHelpers::Average(pixels));
  }
};

#endif
