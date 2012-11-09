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

#ifndef HoleSizeAcceptanceVisitor_HPP
#define HoleSizeAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include <ITKHelpers/ITKHelpers.h>

// ITK
#include "itkImageRegion.h"

/**

 */
template <typename TGraph>
struct HoleSizeAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  Mask* MaskImage;

  const unsigned int HalfWidth;

  float HolePixelRatio;
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  HoleSizeAcceptanceVisitor(Mask* const mask, const unsigned int halfWidth,
                            const float holePixelRatio, const std::string& visitorName = "HoleSizeAcceptanceVisitor") :
  AcceptanceVisitorParent<TGraph>(visitorName), MaskImage(mask),
    HalfWidth(halfWidth), HolePixelRatio(holePixelRatio)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const override
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->HalfWidth);

    unsigned int numberOfHolePixels = this->MaskImage->CountHolePixels(targetRegion);

    float ratio = static_cast<float>(numberOfHolePixels)/static_cast<float>(targetRegion.GetNumberOfPixels());
    std::cout << "Hole pixel ratio: " << ratio << std::endl;
    if(ratio < this->HolePixelRatio)
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
