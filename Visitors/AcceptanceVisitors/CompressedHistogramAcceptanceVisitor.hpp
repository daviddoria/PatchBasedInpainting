#ifndef CompressedHistogramAcceptanceVisitor_HPP
#define CompressedHistogramAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "ImageProcessing/MaskOperations.h"
#include "Helpers/ITKHelpers.h"
#include "Utilities/Histogram.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage>
struct CompressedHistogramAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;

  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  CompressedHistogramAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth,
                                       const float differenceThreshold = 100.0f) :
  AcceptanceVisitorParent<TGraph>("CompressedHistogramAcceptanceVisitor"),
  Image(image), MaskImage(mask), HalfWidth(halfWidth),
  DifferenceThreshold(differenceThreshold)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    float computedEnergy;
    return AcceptMatch(target, source, computedEnergy);
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    // Get the pixels to use in the target region
    std::vector<typename TImage::PixelType> validPixelsTargetRegion =
             MaskOperations::GetValidPixelsInRegion(Image, MaskImage, targetRegion);

    // Get the pixels to use in the source region
    std::vector<itk::Offset<2> > validOffsets = MaskImage->GetValidOffsetsInRegion(targetRegion);
    std::vector<itk::Index<2> > sourcePatchValidPixelIndices =
           ITKHelpers::OffsetsToIndices(validOffsets, sourceRegion.GetIndex());
    std::vector<typename TImage::PixelType> validPixelsSourceRegion =
           ITKHelpers::GetPixelValues(Image, sourcePatchValidPixelIndices);

    assert(validPixelsSourceRegion.size() == validPixelsTargetRegion.size());

    unsigned int numberOfPixels = validPixelsTargetRegion.size();

    float totalHistogramDifference = 0.0f;

    for(unsigned int component = 0; component < Image->GetNumberOfComponentsPerPixel(); ++component)
    {
      std::vector<float> targetValues(numberOfPixels);
      std::vector<float> sourceValues(numberOfPixels);

      for(unsigned int pixelId = 0; pixelId < numberOfPixels; ++pixelId)
      {
        targetValues[pixelId] = validPixelsTargetRegion[pixelId][component];
        sourceValues[pixelId] = validPixelsSourceRegion[pixelId][component];
      }

      float minTargetValue = *(std::min_element(targetValues.begin(), targetValues.end()));
      float minSourceValue = *(std::min_element(sourceValues.begin(), sourceValues.end()));
      
      float maxTargetValue = *(std::max_element(targetValues.begin(), targetValues.end()));
      float maxSourceValue = *(std::max_element(sourceValues.begin(), sourceValues.end()));

      float minValue = std::min(minTargetValue, minSourceValue);
      float maxValue = std::max(maxTargetValue, maxSourceValue);
      
      std::vector<float> targetHistogram = Histogram::ScalarHistogram(targetValues, 20,
                                                                      minValue, maxValue);
      std::vector<float> sourceHistogram = Histogram::ScalarHistogram(sourceValues, 20,
                                                                      minValue, maxValue);

      // We normalize the histograms because the magnitude of the histogram difference should not
      // change based on the number of pixels that were in the valid region of the patches.
      Helpers::NormalizeVector(targetHistogram);
      Helpers::NormalizeVector(sourceHistogram);

      float channelHistogramDifference = Helpers::VectorSumOfAbsoluteDifferences(targetHistogram, sourceHistogram);
      totalHistogramDifference += channelHistogramDifference;
    }

    // Compute the difference
    computedEnergy = totalHistogramDifference;
    std::cout << "HistogramDifferenceAcceptanceVisitor Energy: " << computedEnergy << std::endl;

    if(computedEnergy < DifferenceThreshold)
      {
      std::cout << "HistogramDifferenceAcceptanceVisitor: Match accepted (less than " << DifferenceThreshold
                << ")" << std::endl;
      return true;
      }
    else
      {
      std::cout << "HistogramDifferenceAcceptanceVisitor: Match rejected (greater than " << DifferenceThreshold
                << ")" << std::endl;
      return false;
      }
  };

};

#endif
