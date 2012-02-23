#ifndef SourceTargetVarianceDifferenceAcceptanceVisitor_HPP
#define SourceTargetVarianceDifferenceAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "ImageProcessing/Mask.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/BoostHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage>
struct SourceTargetVarianceDifferenceAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  float DifferenceThreshold;
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  SourceTargetVarianceDifferenceAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, const float differenceThreshold = 100) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), NumberOfFinishedVertices(0), DifferenceThreshold(differenceThreshold)
  {

  }
  
  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    // Compute the variance of the valid pixels in the target region
    std::vector<itk::Index<2> > validPixelsTargetRegion = MaskImage->GetValidPixelsInRegion(targetRegion);
    typename TImage::PixelType targetRegionSourcePixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, validPixelsTargetRegion);

    // Compute the variance of the pixels in the source region corresponding to hole pixels in the target region.
    std::vector<itk::Offset<2> > holeOffsets = MaskImage->GetHoleOffsetsInRegion(targetRegion);
    std::vector<itk::Index<2> > sourcePatchHolePixels = ITKHelpers::OffsetsToIndices(holeOffsets, sourceRegion.GetIndex());
    typename TImage::PixelType sourceRegionTargetPixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, sourcePatchHolePixels);

    // Compute the difference
    computedEnergy = (targetRegionSourcePixelVariance - sourceRegionTargetPixelVariance).GetNorm();
    std::cout << "VarianceDifferenceAcceptanceVisitor Energy: " << computedEnergy << std::endl;

    if(computedEnergy < DifferenceThreshold)
      {
      std::cout << "VarianceDifferenceAcceptanceVisitor: Match accepted (less than " << DifferenceThreshold << ")" << std::endl;
      return true;
      }
    else
      {
      std::cout << "VarianceDifferenceAcceptanceVisitor: Match rejected (greater than " << DifferenceThreshold << ")" << std::endl;
      return false;
      }
  };

};

#endif
