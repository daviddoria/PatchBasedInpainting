/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef VarianceFunctor_HPP
#define VarianceFunctor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "Helpers/Statistics.h"
#include "ITKHelpers/ITKHelpers.h"

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
