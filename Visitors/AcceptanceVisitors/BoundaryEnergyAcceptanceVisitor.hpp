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

#ifndef BoundaryEnergyAcceptanceVisitor_HPP
#define BoundaryEnergyAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Submodules
#include <Mask/Mask.h>
#include <ITKHelpers/ITKHelpers.h>
#include <BoostHelpers/BoostHelpers.h>

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**
  * This visitor writes out information and images at each iteration.
 */
template <typename TGraph, typename TImage, typename TBoundaryStatusMap>
struct BoundaryEnergyAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices = 0;

  TBoundaryStatusMap& BoundaryStatusMap;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  DebugVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth,
               TBoundaryStatusMap& in_boundaryStatusMap) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), BoundaryStatusMap(in_boundaryStatusMap)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    BoundaryEnergy<TImage> boundaryEnergy(Image, MaskImage);

    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    // Compute boundary energy
    float energy = boundaryEnergy(sourceRegion, targetRegion);
    std::cout << "Energy: " << energy << std::endl;

    float energyThreshold = 100;
    if(energy < energyThreshold)
      {
      std::cout << "Match accepted." << std::endl;
      return true;
      }
    else
      {
      std::cout << "Match rejected." << std::endl;
      return false;
      }
  }

};

#endif
