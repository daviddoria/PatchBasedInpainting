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

#ifndef FullPatchAverageDifference_HPP
#define FullPatchAverageDifference_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Submodules
#include <ITKHelpers/ITKHelpers.h>

// ITK
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage>
struct FullPatchAverageDifference : public AcceptanceVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  FullPatchAverageDifference(TImage* const image, const unsigned int halfWidth) :
  Image(image), HalfWidth(halfWidth)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const override
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);
    typename TImage::PixelType targetRegionAverage = ITKHelpers::AverageInRegion(Image, targetRegion);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);
    typename TImage::PixelType sourceRegionAverage = ITKHelpers::AverageInRegion(Image, sourceRegion);

    // Compute the difference
    computedEnergy = (targetRegionAverage - sourceRegionAverage).GetNorm();
    std::cout << "FullPatchAverageDifference Energy: " << computedEnergy << std::endl;
    return true;
  }

private:
  TImage* Image;

  const unsigned int HalfWidth;
};

#endif
