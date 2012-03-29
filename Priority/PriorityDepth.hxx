
#include "PriorityDepth.h"

#include "ImageProcessing/Derivatives.h"
#include "ImageProcessing/Isophotes.h"
#include "Mask/MaskOperations.h"

#include <stdexcept>

template <typename TNode, typename TImage>
PriorityDepth<TNode, TImage>::PriorityDepth(const TImage* const image, const Mask* const maskImage, const unsigned int patchRadius) : MaskImage(maskImage), Image(image)
{
  if(image->GetNumberOfComponentsPerPixel() < 4)
    {
    throw std::runtime_error("Cannot instantiate PriorityDepth with an image with less than 4 channels!");
    }

  this->DepthIsophoteImage = ITKHelpers::FloatVector2ImageType::New();
  ITKHelpers::InitializeImage<ITKHelpers::FloatVector2ImageType>(this->DepthIsophoteImage, image->GetLargestPossibleRegion());

  typedef itk::VectorIndexSelectionCastImageFilter<ITKHelpers::FloatVectorImageType, ITKHelpers::FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(3);
  indexSelectionFilter->SetInput(image);
  indexSelectionFilter->Update();

  //HelpersOutput::WriteImage<FloatScalarImageType>(indexSelectionFilter->GetOutput(), "Debug/Depth.mha");

  this->DepthImage = ITKHelpers::FloatScalarImageType::New();
  //Helpers::MaskedBlur<FloatScalarImageType>(indexSelectionFilter->GetOutput(), maskImage, 2.0f, this->BlurredDepth);

  typedef itk::BilateralImageFilter<ITKHelpers::FloatScalarImageType, ITKHelpers::FloatScalarImageType>  BilateralImageFilterType;
  BilateralImageFilterType::Pointer bilateralImageFilter = BilateralImageFilterType::New();
  bilateralImageFilter->SetInput(indexSelectionFilter->GetOutput());
  float domainSigma = 10.0f;
  bilateralImageFilter->SetDomainSigma(domainSigma);
  float rangeSigma = 10.0f;
  bilateralImageFilter->SetRangeSigma(rangeSigma);
  bilateralImageFilter->Update();

  ITKHelpers::DeepCopy(bilateralImageFilter->GetOutput(), this->DepthImage.GetPointer());

  //HelpersOutput::WriteImage<FloatScalarImageType>(this->BlurredDepth, "Debug/BlurredDepth.mha");

  Isophotes::ComputeMaskedIsophotesInRegion(this->DepthImage.GetPointer(), maskImage, image->GetLargestPossibleRegion(), this->DepthIsophoteImage.GetPointer());
  //HelpersOutput::Write2DVectorImage(this->DepthIsophoteImage, "Debug/BlurredDepthIsophoteImage.mha");

}

// template <typename TImage>
// float PriorityDepth<TImage>::ComputePriority(const itk::Index<2>& queryPixel)
// {
//   float priority = this->DepthIsophoteImage->GetPixel(queryPixel).GetNorm();
//   return priority;
// }

template <typename TNode, typename TImage>
float PriorityDepth<TNode, TImage>::ComputePriority(const TNode& queryPixel) const
{
  ITKHelpers::FloatVector2Type isophote = this->DepthIsophoteImage->GetPixel(queryPixel);
  isophote.Normalize();

  itk::Index<2> pixelAcrossHole = MaskOperations::FindPixelAcrossHole(queryPixel, isophote, this->MaskImage);

  FloatVector2Type acrossIsophote = this->DepthIsophoteImage->GetPixel(pixelAcrossHole);

  //float priority = Helpers::AngleBetween(isophote, acrossIsophote) * isophote.GetNorm() * acrossIsophote.GetNorm();
  float priority = isophote * acrossIsophote; // dot product
  if(priority < 0.0f)
    {
    acrossIsophote *= -1.0f;
    priority = isophote * acrossIsophote; // dot product
    }

  return priority;
}

template <typename TNode, typename TImage>
void PriorityDepth<TNode, TImage>::Update(const TNode& filledPixel)
{
  typedef itk::VectorIndexSelectionCastImageFilter<ITKHelpers::FloatVectorImageType, ITKHelpers::FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(3);
  indexSelectionFilter->SetInput(this->Image);
  indexSelectionFilter->Update();

  MaskOperations::MaskedBlur(indexSelectionFilter->GetOutput(), this->MaskImage, 2.0f, this->DepthImage.GetPointer());
  //Isophotes::ComputeMaskedIsophotesInRegion(indexSelectionFilter->GetOutput(), this->MaskImage, filledRegion, this->DepthIsophoteImage);
}
