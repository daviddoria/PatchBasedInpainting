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

#ifndef HoleHistogramDifferenceAcceptanceVisitor_HPP
#define HoleHistogramDifferenceAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Submodules
#include <Mask/Mask.h>
#include <Mask/MaskOperations.h>
#include <ITKHelpers/ITKHelpers.h>
#include "Utilities/Histogram/Histogram.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage>
struct HoleHistogramDifferenceAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;

  float DifferenceThreshold;

  std::vector<float> Mins;
  std::vector<float> Maxs;
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  HoleHistogramDifferenceAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth,
                                           const std::vector<float>& mins, const std::vector<float>& maxs,
                                           const float differenceThreshold = 100.0f) :
    Image(image), MaskImage(mask), HalfWidth(halfWidth), Mins(mins), Maxs(maxs),
    DifferenceThreshold(differenceThreshold)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    float computedEnergy;
    return AcceptMatch(target, source, computedEnergy);
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const override
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, this->HalfWidth);

    // Compute the average of the valid pixels in the target region
    std::vector<typename TImage::PixelType> targetRegionPixels = MaskOperations::GetValidPixelsInRegion(this->Image, this->MaskImage, targetRegion);

    // The next line is the only thing that changed versus HistogramDifferenceAcceptanceVisitor
    std::vector<itk::Offset<2> > targetRegionHoleOffsets = this->MaskImage->GetHoleOffsetsInRegion(targetRegion);
    std::vector<itk::Index<2> > sourcePatchPixelIndices = ITKHelpers::OffsetsToIndices(targetRegionHoleOffsets, sourceRegion.GetIndex());
    std::vector<typename TImage::PixelType> sourceRegionPixels = ITKHelpers::GetPixelValues(Image, sourcePatchPixelIndices);

    assert(sourceRegionPixels.size() == targetRegionPixels.size());

    float totalHistogramDifference = 0.0f;

    for(unsigned int component = 0; component < Image->GetNumberOfComponentsPerPixel(); ++component)
    {
      std::vector<float> targetValues(targetRegionPixels.size());
      std::vector<float> sourceValues(sourceRegionPixels.size());

      for(unsigned int pixelId = 0; pixelId < targetRegionPixels.size(); ++pixelId)
      {
        targetValues[pixelId] = targetRegionPixels[pixelId][component];
      }

      for(unsigned int pixelId = 0; pixelId < sourceRegionPixels.size(); ++pixelId)
      {
        sourceValues[pixelId] = sourceRegionPixels[pixelId][component];
      }

      typedef HistogramGenerator<float> HistogramGeneratorType;
      typedef HistogramGeneratorType::HistogramType HistogramType;
      unsigned int numberOfBins = 20;

      HistogramType targetHistogram = HistogramGeneratorType::ScalarHistogram(targetValues, numberOfBins,
                                                                              this->Mins[component], this->Maxs[component]);
      HistogramType sourceHistogram = HistogramGeneratorType::ScalarHistogram(sourceValues, numberOfBins,
                                                                              this->Mins[component], this->Maxs[component]);

      // We normalize the histograms because the magnitude of the histogram difference should not change based on the number of pixels that were in the valid region of the patches.
      Helpers::NormalizeVector(targetHistogram);
      Helpers::NormalizeVector(sourceHistogram);

      float channelHistogramDifference = Helpers::VectorSumOfAbsoluteDifferences(targetHistogram, sourceHistogram);
      totalHistogramDifference += channelHistogramDifference;
    }

    // Compute the difference
    computedEnergy = totalHistogramDifference;
    std::cout << "HoleHistogramDifferenceAcceptanceVisitor Energy: " << computedEnergy << std::endl;

    if(computedEnergy < this->DifferenceThreshold)
      {
      std::cout << "HoleHistogramDifferenceAcceptanceVisitor: Match accepted (less than " << this->DifferenceThreshold << ")" << std::endl;
      return true;
      }
    else
      {
      std::cout << "HoleHistogramDifferenceAcceptanceVisitor: Match rejected (greater than " << this->DifferenceThreshold << ")" << std::endl;
      return false;
      }
  }

};

#endif
