#ifndef DilatedVarianceDifferenceAcceptanceVisitor_HPP
#define DilatedVarianceDifferenceAcceptanceVisitor_HPP

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
struct DilatedVarianceDifferenceAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  DilatedVarianceDifferenceAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, const float differenceThreshold = 100) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), NumberOfFinishedVertices(0), DifferenceThreshold(differenceThreshold)
  {
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    // Dilate the hole
    Mask::Pointer targetRegionMask = Mask::New();
    ITKHelpers::ExtractRegion(MaskImage, targetRegion, targetRegionMask.GetPointer());
    
    Mask::Pointer dilatedTargetRegionMask = Mask::New();
    dilatedTargetRegionMask->SetRegions(ITKHelpers::CornerRegion(targetRegion.GetSize()));
    dilatedTargetRegionMask->Allocate();
    ITKHelpers::DilateImage(MaskImage, dilatedTargetRegionMask.GetPointer(), 6);

    // Separate so that only the newly dilated part of the hole remains
    typedef itk::Image<bool, 2> BoolImage;
    BoolImage::Pointer rindImage = BoolImage::New(); // "rind" like an "orange rind"
    ITKHelpers::XORRegions(targetRegionMask.GetPointer(), targetRegionMask->GetLargestPossibleRegion(),
                           dilatedTargetRegionMask.GetPointer(), dilatedTargetRegionMask->GetLargestPossibleRegion(), rindImage.GetPointer());

    std::vector<itk::Index<2> > rindPixels = ITKHelpers::GetPixelsWithValue(dilatedTargetRegionMask.GetPointer(), dilatedTargetRegionMask->GetLargestPossibleRegion(), true);

    std::vector<itk::Offset<2> > rindOffsets = ITKHelpers::IndicesToOffsets(rindPixels, ITKHelpers::ZeroIndex());

    // Compute the variance of the rind pixels in the target region
    std::vector<itk::Index<2> > targetRindPixels = ITKHelpers::OffsetsToIndices(rindOffsets, sourceRegion.GetIndex());
    typename TImage::PixelType targetRegionSourcePixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, targetRindPixels);

    // Compute the variance of the rind pixels in the source region
    std::vector<itk::Index<2> > sourceRindPixels = ITKHelpers::OffsetsToIndices(rindOffsets, sourceRegion.GetIndex());
    typename TImage::PixelType sourceRegionTargetPixelVariance = ITKHelpers::VarianceOfPixelsAtIndices(Image, sourceRindPixels);

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
