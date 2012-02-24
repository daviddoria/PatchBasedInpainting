#ifndef DilatedSourceValidTargetValidAcceptanceVisitor_HPP
#define DilatedSourceValidTargetValidAcceptanceVisitor_HPP

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
template <typename TGraph, typename TImage, typename TFunctor>
struct DilatedSourceValidTargetValidAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  TFunctor Functor;

  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  DilatedSourceValidTargetValidAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, TFunctor functor = TFunctor(), const float differenceThreshold = 100,
    const std::string& visitorName = "DilatedSourceValidTargetValidAcceptanceVisitor") :
  AcceptanceVisitorParent<TGraph>(visitorName),
  Image(image), MaskImage(mask), HalfWidth(halfWidth), NumberOfFinishedVertices(0), Functor(functor), DifferenceThreshold(differenceThreshold)
  {
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    //std::cout << "DilatedVarianceDifferenceAcceptanceVisitor::AcceptMatch" << std::endl;

    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    Mask::Pointer targetRegionMask = Mask::New();
    ITKHelpers::ExtractRegion(MaskImage, targetRegion, targetRegionMask.GetPointer());

    Mask::Pointer dilatedTargetRegionMask = Mask::New();
    ITKHelpers::DilateImage(targetRegionMask.GetPointer(), dilatedTargetRegionMask.GetPointer(), 6);

    typedef itk::Image<bool, 2> BoolImage;
    BoolImage::Pointer rindImage = BoolImage::New(); // "rind" like an "orange rind"
    ITKHelpers::XORRegions(targetRegionMask.GetPointer(), targetRegionMask->GetLargestPossibleRegion(),
                           dilatedTargetRegionMask.GetPointer(), dilatedTargetRegionMask->GetLargestPossibleRegion(), rindImage.GetPointer());

    std::vector<itk::Index<2> > rindPixels = ITKHelpers::GetPixelsWithValue(rindImage.GetPointer(), rindImage->GetLargestPossibleRegion(), true);

    std::vector<itk::Offset<2> > rindOffsets = ITKHelpers::IndicesToOffsets(rindPixels, ITKHelpers::ZeroIndex());

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
  };

};

#endif
