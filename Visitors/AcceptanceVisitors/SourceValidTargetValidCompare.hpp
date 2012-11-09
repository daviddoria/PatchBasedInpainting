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

#ifndef SourceValidTargetValidCompare_HPP
#define SourceValidTargetValidCompare_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "Mask/Mask.h"
#include "ITKHelpers/ITKHelpers.h"
#include "BoostHelpers/BoostHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage, typename TFunctor>
struct SourceValidTargetValidCompare : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices = 0;

  TFunctor Functor;
  
  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  SourceValidTargetValidCompare(TImage* const image, Mask* const mask,
                                const unsigned int halfWidth, TFunctor functor = TFunctor(), const float differenceThreshold = 100,
    const std::string& visitorName = "SourceValidTargetValidCompare") :
  AcceptanceVisitorParent<TGraph>(visitorName),
  Image(image), MaskImage(mask), HalfWidth(halfWidth),
    Functor(functor), DifferenceThreshold(differenceThreshold)
  {
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const override
  {
    //std::cout << "DilatedVarianceDifferenceAcceptanceVisitor::AcceptMatch" << std::endl;

    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, this->HalfWidth);

    std::vector<itk::Offset<2> > validOffsets = this->MaskImage->GetValidOffsetsInRegion(targetRegion);
    
    std::vector<itk::Index<2> > validPixelsIndicesTargetRegion = ITKHelpers::OffsetsToIndices(validOffsets, targetRegion.GetIndex());
    std::vector<typename TImage::PixelType> validPixelsTargetRegion = ITKHelpers::GetPixelValues(Image, validPixelsIndicesTargetRegion);

    typename TypeTraits<typename TImage::PixelType>::LargerType targetValue = Functor(validPixelsTargetRegion);
    
    std::vector<itk::Index<2> > validPixelsIndicesSourceRegion = ITKHelpers::OffsetsToIndices(validOffsets, sourceRegion.GetIndex());
    std::vector<typename TImage::PixelType> validPixelsSourceRegion = ITKHelpers::GetPixelValues(Image, validPixelsIndicesSourceRegion);

    typename TypeTraits<typename TImage::PixelType>::LargerType sourceValue = Functor(validPixelsSourceRegion);

    typename TypeTraits<typename TImage::PixelType>::LargerType difference = targetValue - sourceValue;
    // std::cout << this->VisitorName << " Vector of energies: : " << difference << std::endl;
    
    // Compute the difference
    //computedEnergy = (targetValue - sourceValue).GetNorm();
    computedEnergy = ITKHelpers::SumOfComponentMagnitudes(difference);

    // std::cout << this->VisitorName << " Energy: " << computedEnergy << std::endl;

    if(computedEnergy < this->DifferenceThreshold)
    {
      std::cout << this->VisitorName << ": Match accepted (" << computedEnergy
                << " is less than " << this->DifferenceThreshold << ")" << std::endl << std::endl;
      return true;
    }
    else
    {
      std::cout << this->VisitorName << ": Match rejected (" << computedEnergy
                << " is greater than " << this->DifferenceThreshold << ")" << std::endl << std::endl;
      return false;
    }
  }

};

#endif
