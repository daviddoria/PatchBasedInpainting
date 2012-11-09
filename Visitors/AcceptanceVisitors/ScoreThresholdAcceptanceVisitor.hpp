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

#ifndef ScoreThresholdAcceptanceVisitor_HPP
#define ScoreThresholdAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "ITKHelpers/ITKHelpers.h"

// ITK
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TPropertyMap, typename TDifferenceFunction>
struct ScoreThresholdAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  Mask* MaskImage;

  const unsigned int HalfWidth;

  TPropertyMap PropertyMap;

  float Threshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  ScoreThresholdAcceptanceVisitor(Mask* const mask, const unsigned int halfWidth, TPropertyMap propertyMap,
                                  const float threshold, const std::string& visitorName = "HoleSizeAcceptanceVisitor") :
  AcceptanceVisitorParent<TGraph>(visitorName), MaskImage(mask), HalfWidth(halfWidth), PropertyMap(propertyMap),
  Threshold(threshold)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const override
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->HalfWidth);

    TDifferenceFunction differenceFunction;
    float difference = differenceFunction(get(this->PropertyMap, target), get(this->PropertyMap, source));
    
    std::cout << "ScoreThresholdAcceptanceVisitor difference: " << difference << std::endl;
    if(difference < this->Threshold)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

};

#endif
