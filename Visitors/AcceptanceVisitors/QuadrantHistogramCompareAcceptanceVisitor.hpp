#ifndef QuadrantHistogramCompareAcceptanceVisitor_HPP
#define QuadrantHistogramCompareAcceptanceVisitor_HPP

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
struct QuadrantHistogramCompareAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;

  const unsigned int Quadrant;
  
  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  QuadrantHistogramCompareAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth,
                                            const unsigned int quadrant, const float differenceThreshold = 100.0f) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), Quadrant(quadrant), DifferenceThreshold(differenceThreshold)
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

    itk::ImageRegion<2> targetRegionQuadrant = ITKHelpers::GetQuadrant(targetRegion, this->Quadrant);
    itk::ImageRegion<2> sourceRegionQuadrant = ITKHelpers::GetQuadrant(sourceRegion, this->Quadrant);

    std::vector<typename TImage::PixelType> validPixelsTargetRegion = MaskOperations::GetValidPixelsInRegion(Image, MaskImage, targetRegionQuadrant);

    // If less than 20 percent of the quadrant has valid pixels, the comparison is too specific and the energy will often be erroneously high
    float quadrantCoverage = static_cast<float>(validPixelsTargetRegion.size()) / static_cast<float>(targetRegionQuadrant.GetNumberOfPixels());
    if(quadrantCoverage < .2)
    {
      std::cout << "Skipping quadrant " << Quadrant << " (coverage " << quadrantCoverage << ")" << std::endl;
      computedEnergy = 0.0f;
      return true;
    }
    std::vector<itk::Offset<2> > validOffsets = MaskImage->GetValidOffsetsInRegion(targetRegionQuadrant);
    std::vector<itk::Index<2> > sourcePatchValidPixelIndices = ITKHelpers::OffsetsToIndices(validOffsets, sourceRegionQuadrant.GetIndex());
    std::vector<typename TImage::PixelType> validPixelsSourceRegion = ITKHelpers::GetPixelValues(Image, sourcePatchValidPixelIndices);

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

      std::vector<float> targetHistogram = Histogram::ScalarHistogram(targetValues, 20);
      std::vector<float> sourceHistogram = Histogram::ScalarHistogram(sourceValues, 20);

      // We normalize the histograms because the magnitude of the histogram difference should not change based on the number of pixels that were in the valid region of the patches.
      Helpers::NormalizeVector(targetHistogram);
      Helpers::NormalizeVector(sourceHistogram);

      float channelHistogramDifference = Helpers::VectorSumOfAbsoluteDifferences(targetHistogram, sourceHistogram);
      totalHistogramDifference += channelHistogramDifference;
    }

    // Compute the difference
    computedEnergy = totalHistogramDifference;
    std::cout << "QuadrantHistogramCompareAcceptanceVisitor Energy: " << computedEnergy << std::endl;

    if(computedEnergy < DifferenceThreshold)
      {
      std::cout << "QuadrantHistogramCompareAcceptanceVisitor: Match accepted (less than " << DifferenceThreshold << ")" << std::endl;
      return true;
      }
    else
      {
      std::cout << "QuadrantHistogramCompareAcceptanceVisitor: Match rejected (greater than " << DifferenceThreshold << ")" << std::endl;
      return false;
      }
  };

};

#endif
