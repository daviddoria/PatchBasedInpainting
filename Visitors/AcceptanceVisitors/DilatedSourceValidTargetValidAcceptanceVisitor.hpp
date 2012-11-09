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

#ifndef DilatedSourceValidTargetValidAcceptanceVisitor_HPP
#define DilatedSourceValidTargetValidAcceptanceVisitor_HPP

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
struct DilatedSourceValidTargetValidAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices = 0;

  TFunctor Functor;

  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  DilatedSourceValidTargetValidAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, TFunctor functor = TFunctor(), const float differenceThreshold = 100,
    const std::string& visitorName = "DilatedSourceValidTargetValidAcceptanceVisitor") :
  AcceptanceVisitorParent<TGraph>(visitorName),
  Image(image), MaskImage(mask), HalfWidth(halfWidth), Functor(functor), DifferenceThreshold(differenceThreshold)
  {
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const override
  {
    //std::cout << "DilatedVarianceDifferenceAcceptanceVisitor::AcceptMatch" << std::endl;

    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    Mask::Pointer targetRegionMask = Mask::New();
    ITKHelpers::ExtractRegion(MaskImage, targetRegion, targetRegionMask.GetPointer());

    Mask::Pointer dilatedTargetRegionMask = Mask::New();
    ITKHelpers::DilateImage(targetRegionMask.GetPointer(), 6, dilatedTargetRegionMask.GetPointer());

    typedef itk::Image<bool, 2> BoolImage;
    BoolImage::Pointer rindImage = BoolImage::New(); // "rind" like an "orange rind"
    ITKHelpers::XORRegions(targetRegionMask.GetPointer(), targetRegionMask->GetLargestPossibleRegion(),
                           dilatedTargetRegionMask.GetPointer(), dilatedTargetRegionMask->GetLargestPossibleRegion(), rindImage.GetPointer());

    std::vector<itk::Index<2> > rindPixels =
        ITKHelpers::GetPixelsWithValueInRegion(rindImage.GetPointer(),
                                               rindImage->GetLargestPossibleRegion(), true);

    itk::Index<2> zeroIndex = {{0,0}};
    std::vector<itk::Offset<2> > rindOffsets = ITKHelpers::IndicesToOffsets(rindPixels, zeroIndex);

    std::vector<itk::Index<2> > targetRindPixelIndices = ITKHelpers::OffsetsToIndices(rindOffsets, targetRegion.GetIndex());
    std::vector<typename TImage::PixelType> targetRindPixels = ITKHelpers::GetPixelValues(Image, targetRindPixelIndices);
    typename TypeTraits<typename TImage::PixelType>::LargerType targetValue = Functor(targetRindPixels);

    std::vector<itk::Index<2> > sourceRindPixelIndices = ITKHelpers::OffsetsToIndices(rindOffsets, sourceRegion.GetIndex());
    std::vector<typename TImage::PixelType> sourceRindPixels = ITKHelpers::GetPixelValues(Image, sourceRindPixelIndices);
    typename TypeTraits<typename TImage::PixelType>::LargerType sourceValue = Functor(sourceRindPixels);

    // Compute the difference
    computedEnergy = (targetValue - sourceValue).GetNorm();
    //std::cout << this->VisitorName << " Energy: " << computedEnergy << std::endl;

    if(computedEnergy < DifferenceThreshold)
      {
      std::cout << this->VisitorName << ": Match accepted (" << computedEnergy << " is than " << DifferenceThreshold << ")" << std::endl << std::endl;
      return true;
      }
    else
      {
      std::cout << this->VisitorName << ": Match rejected (" << computedEnergy << " is greater than " << DifferenceThreshold << ")" << std::endl << std::endl;
      return false;
      }
  }

};

#endif
