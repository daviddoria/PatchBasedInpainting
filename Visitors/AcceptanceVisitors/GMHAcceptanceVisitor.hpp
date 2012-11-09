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

#ifndef GMHAcceptanceVisitor_HPP
#define GMHAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

#include "DifferenceFunctions/GMHDifference.hpp"

// Submodules
#include <Mask/Mask.h>

/** Gradient Magnitude Histogram acceptance visitor.
  */
template <typename TGraph, typename TImage>
struct GMHAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  GMHAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth,
                       const float distanceThreshold, const unsigned int numberOfBinsPerChannel) :
    Image(image), MaskImage(mask), HalfWidth(halfWidth), DistanceThreshold(distanceThreshold),
    NumberOfBinsPerChannel(numberOfBinsPerChannel)
  {

  }

  /** This version does not allow the caller to get the output value. */
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    float computedEnergy = 0.0f;
    return AcceptMatch(target, source, computedEnergy);
  }
  
  /** This version allows the caller to get the output value. */
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy = 0.0f) const override
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, this->HalfWidth);

    GMHDifference<TImage> gmhDifference(this->Image, this->MaskImage, this->NumberOfBinsPerChannel);

    computedEnergy = gmhDifference.Difference(targetRegion, sourceRegion);

    if(computedEnergy < this->DistanceThreshold)
    {
      std::cout << "GMHAcceptanceVisitor passed with distance: " << computedEnergy << std::endl;
      return true;
    }

    std::cout << "GMHAcceptanceVisitor failed with distance: " << computedEnergy << std::endl;
    return false;
  }

private:

  TImage* Image;
  Mask* MaskImage;
  unsigned int HalfWidth;
  float DistanceThreshold;
  unsigned int NumberOfBinsPerChannel;
};

#endif
