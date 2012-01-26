
#include "PriorityDepth.h"

#include "ImageProcessing/Derivatives.h"
#include "Helpers/HelpersOutput.h"
#include "ImageProcessing/Isophotes.h"
#include "ImageProcessing/MaskOperations.h"

#include <stdexcept>

template <typename TImage>
PriorityDepth<TImage>::PriorityDepth(const TImage* image, const Mask* maskImage, unsigned int patchRadius) : MaskImage(maskImage), Image(image)
{
  if(image->GetNumberOfComponentsPerPixel() < 4)
    {
    throw std::runtime_error("Cannot instantiate PriorityDepth with an image with less than 4 channels!");
    }

  this->DepthIsophoteImage = FloatVector2ImageType::New();
  ITKHelpers::InitializeImage<FloatVector2ImageType>(this->DepthIsophoteImage, image->GetLargestPossibleRegion());

  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(3);
  indexSelectionFilter->SetInput(image);
  indexSelectionFilter->Update();

  //HelpersOutput::WriteImage<FloatScalarImageType>(indexSelectionFilter->GetOutput(), "Debug/Depth.mha");

  this->BlurredDepth = FloatScalarImageType::New();
  //Helpers::MaskedBlur<FloatScalarImageType>(indexSelectionFilter->GetOutput(), maskImage, 2.0f, this->BlurredDepth);

  typedef itk::BilateralImageFilter<FloatScalarImageType, FloatScalarImageType>  BilateralImageFilterType;
  BilateralImageFilterType::Pointer bilateralImageFilter = BilateralImageFilterType::New();
  bilateralImageFilter->SetInput(indexSelectionFilter->GetOutput());
  float domainSigma = 10.0f;
  bilateralImageFilter->SetDomainSigma(domainSigma);
  float rangeSigma = 10.0f;
  bilateralImageFilter->SetRangeSigma(rangeSigma);
  bilateralImageFilter->Update();

  ITKHelpers::DeepCopy<FloatScalarImageType>(bilateralImageFilter->GetOutput(), this->BlurredDepth);

  //HelpersOutput::WriteImage<FloatScalarImageType>(this->BlurredDepth, "Debug/BlurredDepth.mha");

  Isophotes::ComputeMaskedIsophotesInRegion(this->BlurredDepth, maskImage, image->GetLargestPossibleRegion(), this->DepthIsophoteImage);
  //HelpersOutput::Write2DVectorImage(this->DepthIsophoteImage, "Debug/BlurredDepthIsophoteImage.mha");

}

// template <typename TImage>
// float PriorityDepth<TImage>::ComputePriority(const itk::Index<2>& queryPixel)
// {
//   float priority = this->DepthIsophoteImage->GetPixel(queryPixel).GetNorm();
//   return priority;
// }

template <typename TImage>
float PriorityDepth<TImage>::ComputePriority(const itk::Index<2>& queryPixel) const
{
  FloatVector2Type isophote = this->DepthIsophoteImage->GetPixel(queryPixel);
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

template <typename TImage>
void PriorityDepth<TImage>::Update(const itk::Index<2>& filledPixel)
{
  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(3);
  indexSelectionFilter->SetInput(this->Image);
  indexSelectionFilter->Update();

  MaskOperations::MaskedBlur<FloatScalarImageType>(indexSelectionFilter->GetOutput(), this->MaskImage, 2.0f, this->BlurredDepth);
  //Isophotes::ComputeMaskedIsophotesInRegion(indexSelectionFilter->GetOutput(), this->MaskImage, filledRegion, this->DepthIsophoteImage);
}
