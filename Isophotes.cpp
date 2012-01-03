#include "Isophotes.h"

#include "Derivatives.h"
#include "Helpers.h"

#include "itkRGBToLuminanceImageFilter.h"

namespace Isophotes
{
void ComputeColorIsophotesInRegion(const FloatVectorImageType* image, const Mask* mask,
                                   const itk::ImageRegion<2>& region , FloatVector2ImageType* isophotes)
{
  //EnterFunction("ComputeIsophotes()");
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  Helpers::VectorImageToRGBImage(image, rgbImage);

  //HelpersOutput::WriteImageConditional<RGBImageType>(rgbImage, "Debug/Initialize.rgb.mha", this->DebugImages);

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  FloatScalarImageType::Pointer luminanceImage = FloatScalarImageType::New();
  Helpers::DeepCopy<FloatScalarImageType>(luminanceFilter->GetOutput(), luminanceImage);

  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel. From TestIsophotes.cpp, it actually seems like not blurring, but using a masked sobel operator produces the most reliable isophotes.
  unsigned int kernelRadius = 0;
  Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), mask, kernelRadius, blurredLuminance);

  //HelpersOutput::WriteImageConditional<FloatScalarImageType>(blurredLuminance, "Debug/Initialize.blurredLuminance.mha", true);

  Helpers::InitializeImage<FloatVector2ImageType>(isophotes, image->GetLargestPossibleRegion());
  Derivatives::ComputeMaskedIsophotesInRegion(blurredLuminance, mask, region, isophotes);

//   if(this->DebugImages)
//     {
//     HelpersOutput::Write2DVectorImage(this->IsophoteImage, "Debug/Initialize.IsophoteImage.mha");
//     }
  //LeaveFunction("ComputeIsophotes()");
}
} // end namespace
