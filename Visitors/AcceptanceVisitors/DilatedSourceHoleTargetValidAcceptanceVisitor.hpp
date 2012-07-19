#ifndef DilatedSourceHoleTargetValidAcceptanceVisitor_HPP
#define DilatedSourceHoleTargetValidAcceptanceVisitor_HPP

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
struct DilatedSourceHoleTargetValidAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  TFunctor Functor;

  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  DilatedSourceHoleTargetValidAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, TFunctor functor = TFunctor(), const float differenceThreshold = 100,
    const std::string& visitorName = "DilatedSourceHoleTargetValidAcceptanceVisitor") :
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

    // Compute the functor on the pixels in the region just inside the hole in the source patch
    typename TypeTraits<typename TImage::PixelType>::LargerType sourceValue;
    {
    Mask::Pointer originalHole = Mask::New();
    ITKHelpers::ExtractRegion(MaskImage, targetRegion, originalHole.GetPointer());

    Mask::Pointer shrunkHole = Mask::New();
    shrunkHole->DeepCopyFrom(originalHole);
    shrunkHole->ShrinkHole(5);

    typedef itk::Image<bool, 2> BoolImage;
    BoolImage::Pointer holeRindImage = BoolImage::New(); // "rind" like an "orange rind"
    ITKHelpers::XORRegions(originalHole.GetPointer(), originalHole->GetLargestPossibleRegion(),
                           shrunkHole.GetPointer(), shrunkHole->GetLargestPossibleRegion(), holeRindImage.GetPointer());

    std::vector<itk::Index<2> > holeRindPixels = ITKHelpers::GetPixelsWithValue(holeRindImage.GetPointer(), holeRindImage->GetLargestPossibleRegion(), true);

    itk::Index<2> zeroIndex = {{0,0}};
    std::vector<itk::Offset<2> > holeRindOffsets = ITKHelpers::IndicesToOffsets(holeRindPixels, zeroIndex);

    std::vector<itk::Index<2> > sourceRindPixelIndices = ITKHelpers::OffsetsToIndices(holeRindOffsets, sourceRegion.GetIndex());
    std::vector<typename TImage::PixelType> sourceRindPixels = ITKHelpers::GetPixelValues(Image, sourceRindPixelIndices);
    sourceValue = Functor(sourceRindPixels);
    }

    // Compute the functor on the pixels in the region just outside the hole in the target patch
    typename TypeTraits<typename TImage::PixelType>::LargerType targetValue;
    {
    Mask::Pointer originalHole = Mask::New();
    ITKHelpers::ExtractRegion(MaskImage, targetRegion, originalHole.GetPointer());

    Mask::Pointer expandedHole = Mask::New();
    expandedHole->DeepCopyFrom(originalHole);
    expandedHole->ExpandHole(5);

    typedef itk::Image<bool, 2> BoolImage;
    BoolImage::Pointer validRindImage = BoolImage::New(); // "rind" like an "orange rind"
    ITKHelpers::XORRegions(originalHole.GetPointer(), originalHole->GetLargestPossibleRegion(),
                           expandedHole.GetPointer(), expandedHole->GetLargestPossibleRegion(), validRindImage.GetPointer());

    std::vector<itk::Index<2> > validRindPixels = ITKHelpers::GetPixelsWithValue(validRindImage.GetPointer(), validRindImage->GetLargestPossibleRegion(), true);

    itk::Index<2> zeroIndex = {{0,0}};
    std::vector<itk::Offset<2> > validRindOffsets = ITKHelpers::IndicesToOffsets(validRindPixels, zeroIndex);

    std::vector<itk::Index<2> > targetRindPixelIndices = ITKHelpers::OffsetsToIndices(validRindOffsets, targetRegion.GetIndex());
    std::vector<typename TImage::PixelType> targetRindPixels = ITKHelpers::GetPixelValues(Image, targetRindPixelIndices);
    targetValue = Functor(targetRindPixels);
    }

    // Compute the difference
    computedEnergy = (targetValue - sourceValue).GetNorm();
    //std::cout << this->VisitorName << " Energy: " << computedEnergy << std::endl;

    if(computedEnergy < DifferenceThreshold)
      {
      std::cout << this->VisitorName << ": Match accepted (" << computedEnergy << " is less than " << DifferenceThreshold << ")" << std::endl << std::endl;
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
