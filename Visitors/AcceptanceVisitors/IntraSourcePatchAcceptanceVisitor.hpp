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

#ifndef IntraSourcePatchAcceptanceVisitor_HPP
#define IntraSourcePatchAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include <Mask/Mask.h>
#include <ITKHelpers/ITKHelpers.h>

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage>
struct IntraSourcePatchAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices = 0;

  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  IntraSourcePatchAcceptanceVisitor(TImage* const image, Mask* const mask,
                                    const unsigned int halfWidth, const float differenceThreshold = 100) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), DifferenceThreshold(differenceThreshold)
  {

  }
  
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const override
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, this->HalfWidth);

    // Compute the variance of the valid pixels in the source region
    std::vector<itk::Offset<2> > validOffsets = this->MaskImage->GetValidOffsetsInRegion(targetRegion);
    std::vector<itk::Index<2> > sourcePatchValidPixels = ITKHelpers::OffsetsToIndices(validOffsets, sourceRegion.GetIndex());
    typename TImage::PixelType sourceRegionSourcePixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, sourcePatchValidPixels);

    // Compute the variance of the target pixels in the source region.
    std::vector<itk::Offset<2> > holeOffsets = this->MaskImage->GetHoleOffsetsInRegion(targetRegion);
    std::vector<itk::Index<2> > sourcePatchHolePixels = ITKHelpers::OffsetsToIndices(holeOffsets, sourceRegion.GetIndex());
    typename TImage::PixelType sourceRegionHolePixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, sourcePatchHolePixels);

    // Compute the difference
    computedEnergy = (sourceRegionSourcePixelVariance - sourceRegionHolePixelVariance).GetNorm();
    std::cout << "IntraSourcePatchAcceptanceVisitor Energy: " << computedEnergy << std::endl;

    if(computedEnergy < this->DifferenceThreshold)
      {
      std::cout << "IntraSourcePatchAcceptanceVisitor: Match accepted (less than " << this->DifferenceThreshold << ")" << std::endl;
      return true;
      }
    else
      {
      std::cout << "IntraSourcePatchAcceptanceVisitor: Match rejected (greater than " << this->DifferenceThreshold << ")" << std::endl;
      return false;
      }
  };

};

#endif
