#ifndef QuadrantHistogramCompareAcceptanceVisitor_HPP
#define QuadrantHistogramCompareAcceptanceVisitor_HPP

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
  Accept a patch if the score of the histogram comparison of the unmasked region of a specified quadrant is
  less than a specified threshold.
 */
template <typename TGraph, typename TImage>
struct QuadrantHistogramCompareAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;

  const unsigned int Quadrant;
  
  std::vector<float> Mins;
  std::vector<float> Maxs;

  float DifferenceThreshold;
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  QuadrantHistogramCompareAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth,
                                            const unsigned int quadrant, const std::vector<float>& mins,
                                            const std::vector<float>& maxs, const float differenceThreshold = 100.0f) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), Quadrant(quadrant), Mins(mins), Maxs(maxs),
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
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, this->HalfWidth);

    itk::ImageRegion<2> targetRegionQuadrant = ITKHelpers::GetQuadrant(targetRegion, this->Quadrant);
    itk::ImageRegion<2> sourceRegionQuadrant = ITKHelpers::GetQuadrant(sourceRegion, this->Quadrant);

    std::vector<typename TImage::PixelType> validPixelsTargetRegion =
              MaskOperations::GetValidPixelsInRegion(this->Image, this->MaskImage, targetRegionQuadrant);

    // If less than 20 percent of the quadrant has valid pixels, the comparison is too specific
    // and the energy will often be erroneously high
    float quadrantCoverage = static_cast<float>(validPixelsTargetRegion.size()) /
                             static_cast<float>(targetRegionQuadrant.GetNumberOfPixels());
    if(quadrantCoverage < .2)
    {
      std::cout << "Skipping quadrant " << this->Quadrant << " (coverage " << quadrantCoverage << ")" << std::endl;
      computedEnergy = 0.0f;
      return true;
    }
    std::vector<itk::Offset<2> > validOffsets = this->MaskImage->GetValidOffsetsInRegion(targetRegionQuadrant);
    std::vector<itk::Index<2> > sourcePatchValidPixelIndices =
            ITKHelpers::OffsetsToIndices(validOffsets, sourceRegionQuadrant.GetIndex());
    std::vector<typename TImage::PixelType> validPixelsSourceRegion =
            ITKHelpers::GetPixelValues(Image, sourcePatchValidPixelIndices);

    assert(validPixelsSourceRegion.size() == validPixelsTargetRegion.size());

    unsigned int numberOfPixels = validPixelsTargetRegion.size();

//    std::cout << "There are " << numberOfPixels << " pixels." << std::endl;
    
    float totalHistogramDifference = 0.0f;
//     std::cout << "There are " << Image->GetNumberOfComponentsPerPixel() << " components in the image." << std::endl;
//     std::cout << "There are " << validPixelsTargetRegion[0].Size() << " components in the target pixels." << std::endl;
//     std::cout << "There are " << validPixelsSourceRegion[0].Size() << " components in the source pixels." << std::endl;
    
    for(unsigned int component = 0; component < Image->GetNumberOfComponentsPerPixel(); ++component)
    {
      // std::cout << "QuadrantHistogramCompareAcceptanceVisitor component " << component << "..." << std::endl;
      std::vector<float> targetValues(numberOfPixels);
      std::vector<float> sourceValues(numberOfPixels);

      for(unsigned int pixelId = 0; pixelId < numberOfPixels; ++pixelId)
      {
        targetValues[pixelId] = validPixelsTargetRegion[pixelId][component];
        sourceValues[pixelId] = validPixelsSourceRegion[pixelId][component];
      }

      typedef HistogramGenerator<float> HistogramGeneratorType;
      typedef HistogramGeneratorType::HistogramType HistogramType;
      unsigned int numberOfBins = 20;

      HistogramType targetHistogram = HistogramGeneratorType::ScalarHistogram(targetValues, numberOfBins,
                                                                              this->Mins[component], this->Maxs[component]);
      HistogramType sourceHistogram = HistogramGeneratorType::ScalarHistogram(sourceValues, numberOfBins,
                                                                              this->Mins[component], this->Maxs[component]);

//       std::cout << "targetHistogram size " << targetHistogram.size() << std::endl;
//       std::cout << "sourceHistogram size " << sourceHistogram.size() << std::endl;

      // We normalize the histograms because the magnitude of the histogram difference
      // should not change based on the number of pixels that were in the valid region of the patches.
      Helpers::NormalizeVector(targetHistogram);
      Helpers::NormalizeVector(sourceHistogram);

      float channelHistogramDifference = Helpers::VectorSumOfAbsoluteDifferences(targetHistogram, sourceHistogram);
      totalHistogramDifference += channelHistogramDifference;
    }

    // Compute the difference
    computedEnergy = totalHistogramDifference;
    std::cout << "QuadrantHistogramCompareAcceptanceVisitor Energy: " << computedEnergy << std::endl;

    if(computedEnergy < this->DifferenceThreshold)
      {
      std::cout << "QuadrantHistogramCompareAcceptanceVisitor: Match accepted (less than "
                << this->DifferenceThreshold << ")" << std::endl;
      return true;
      }
    else
      {
      std::cout << "QuadrantHistogramCompareAcceptanceVisitor: Match rejected (greater than "
                << this->DifferenceThreshold << ")" << std::endl;
      return false;
      }
  }

};

#endif
