#ifndef CorrelationAcceptanceVisitor_HPP
#define CorrelationAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"
#include "Visitors/AcceptanceVisitors/VarianceFunctor.hpp"
#include "Visitors/AcceptanceVisitors/AverageFunctor.hpp"

// Custom
#include "Helpers/ITKHelpers.h"

// ITK
#include "itkAddImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage>
struct CorrelationAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;

  float Threshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  CorrelationAcceptanceVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, const float threshold, const std::string& visitorName = "CorrelationAcceptanceVisitor") :
  AcceptanceVisitorParent<TGraph>(visitorName), Image(image), MaskImage(mask), HalfWidth(halfWidth), Threshold(threshold)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    typedef itk::Image<float, 2> FloatImageType;
    
    typedef itk::VectorMagnitudeImageFilter<TImage, FloatImageType> VectorMagnitudeFilterType;
    typename VectorMagnitudeFilterType::Pointer magnitudeFilter = VectorMagnitudeFilterType::New();
    magnitudeFilter->SetInput(Image);
    magnitudeFilter->Update();

    std::vector<itk::Offset<2> > validOffsets = MaskImage->GetValidOffsetsInRegion(targetRegion);

    FloatImageType::Pointer sourceImage = FloatImageType::New();
//     sourceImage->SetRegions(ITKHelpers::CornerRegion(sourceRegion.GetSize()));
//     sourceImage->Allocate();
    ITKHelpers::ExtractRegion(magnitudeFilter->GetOutput(), sourceRegion, sourceImage.GetPointer());

    FloatImageType::Pointer targetImage = FloatImageType::New();
//     sourceImage->SetRegions(ITKHelpers::CornerRegion(targetRegion.GetSize()));
//     sourceImage->Allocate();
    ITKHelpers::ExtractRegion(magnitudeFilter->GetOutput(), targetRegion, targetImage.GetPointer());

    std::vector<itk::Index<2> > validIndices = ITKHelpers::OffsetsToIndices(validOffsets);

    VarianceFunctor varianceFunctor;
    AverageFunctor averageFunctor;
    /////////// Target region //////////
    std::vector<FloatImageType::PixelType> validPixelsTargetRegion = ITKHelpers::GetPixelValues(targetImage.GetPointer(), validIndices);
    typename TypeTraits<FloatImageType::PixelType>::LargerType targetMean = averageFunctor(validPixelsTargetRegion);
    typename TypeTraits<FloatImageType::PixelType>::LargerType targetStandardDeviation = sqrt(varianceFunctor(validPixelsTargetRegion));

    typedef itk::AddImageFilter <FloatImageType, FloatImageType, FloatImageType> AddImageFilterType;
    AddImageFilterType::Pointer targetAddImageFilter = AddImageFilterType::New();
    targetAddImageFilter->SetInput(targetImage);
    targetAddImageFilter->SetConstant2(-1.0f * targetMean);
    targetAddImageFilter->Update();

    typedef itk::MultiplyImageFilter<FloatImageType, FloatImageType, FloatImageType> MultiplyImageFilterType;
    MultiplyImageFilterType::Pointer targetMultiplyImageFilter = MultiplyImageFilterType::New();
    targetMultiplyImageFilter->SetInput(targetImage);
    targetMultiplyImageFilter->SetConstant(1.0f/targetStandardDeviation);
    targetMultiplyImageFilter->Update();

    /////////// Source region //////////
    std::vector<FloatImageType::PixelType> validPixelsSourceRegion = ITKHelpers::GetPixelValues(sourceImage.GetPointer(), validIndices);
    typename TypeTraits<FloatImageType::PixelType>::LargerType sourceMean = averageFunctor(validPixelsSourceRegion);
    typename TypeTraits<FloatImageType::PixelType>::LargerType sourceStandardDeviation = sqrt(varianceFunctor(validPixelsSourceRegion));

    AddImageFilterType::Pointer sourceAddImageFilter = AddImageFilterType::New();
    sourceAddImageFilter->SetInput(sourceImage);
    sourceAddImageFilter->SetConstant2(-1.0f * sourceMean);
    sourceAddImageFilter->Update();

    MultiplyImageFilterType::Pointer sourceMultiplyImageFilter = MultiplyImageFilterType::New();
    sourceMultiplyImageFilter->SetInput(sourceImage);
    sourceMultiplyImageFilter->SetConstant(1.0f/sourceStandardDeviation);
    sourceMultiplyImageFilter->Update();

    // Initialize
    computedEnergy = 0.0f;
    
    for(std::vector<itk::Index<2> >::const_iterator iter = validIndices.begin(); iter != validIndices.end(); ++iter)
    {
      computedEnergy += (sourceMultiplyImageFilter->GetOutput()->GetPixel(*iter) * targetMultiplyImageFilter->GetOutput()->GetPixel(*iter));
    }

    computedEnergy /= static_cast<float>(validIndices.size());

    if(computedEnergy < Threshold)
      {
      std::cout << this->VisitorName << ": Match accepted (" << computedEnergy << " is less than " << Threshold << ")" << std::endl << std::endl;
      return true;
      }
    else
      {
      std::cout << this->VisitorName << ": Match rejected (" << computedEnergy << " is greater than " << Threshold << ")" << std::endl << std::endl;
      return false;
      }
  };

};

#endif
