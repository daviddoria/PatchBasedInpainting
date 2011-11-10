#include "CriminisiInpainting.h"

#include "HelpersOutput.h"

#include <boost/bind.hpp>

void CriminisiInpainting::SetPatchSearchFunctionToScaleConsistent()
{
  this->PatchSearchFunction = boost::bind(&CriminisiInpainting::FindBestPatchScaleConsistent,this,_1,_2);
}

void CriminisiInpainting::SetPatchSearchFunctionToNormal()
{
  this->PatchSearchFunction = boost::bind(&CriminisiInpainting::FindBestPatchNormal,this,_1,_2);
}

void CriminisiInpainting::SetPatchSearchFunctionToTwoStepDepth()
{
  this->PatchSearchFunction = boost::bind(&CriminisiInpainting::FindBestPatchTwoStepDepth,this,_1,_2);
}

FloatVectorImageType::Pointer CriminisiInpainting::GetCurrentOutputImage()
{
  return this->CurrentOutputImage;
}

FloatScalarImageType::Pointer CriminisiInpainting::GetPriorityImage()
{
    return this->PriorityImage;
}

FloatScalarImageType::Pointer CriminisiInpainting::GetConfidenceImage()
{
  return this->ConfidenceImage;
}

FloatScalarImageType::Pointer CriminisiInpainting::GetConfidenceMapImage()
{
  return this->ConfidenceMapImage;
}

UnsignedCharScalarImageType::Pointer CriminisiInpainting::GetBoundaryImage()
{
  return this->BoundaryImage;
}

Mask::Pointer CriminisiInpainting::GetMaskImage()
{
  return this->CurrentMask;
}

FloatVector2ImageType::Pointer CriminisiInpainting::GetBoundaryNormalsImage()
{
  return this->BoundaryNormals;
}

FloatVector2ImageType::Pointer CriminisiInpainting::GetIsophoteImage()
{
  return this->IsophoteImage;
}

FloatScalarImageType::Pointer CriminisiInpainting::GetDataImage()
{
  return this->DataImage;
}

void CriminisiInpainting::SetPatchRadius(const unsigned int radius)
{
  // Since this is the radius of the patch, there are no restrictions for the radius to be odd or even.
  this->PatchRadius.Fill(radius);
}

void CriminisiInpainting::SetMaxForwardLookPatches(const unsigned int numberOfPatches)
{
  this->MaxForwardLookPatches = numberOfPatches;
}

void CriminisiInpainting::SetNumberOfTopPatchesToSave(const unsigned int number)
{
  this->NumberOfTopPatchesToSave = number;
}

void CriminisiInpainting::SetImage(const FloatVectorImageType::Pointer image)
{
  // Store the original image
  Helpers::DeepCopy<FloatVectorImageType>(image, this->OriginalImage);

  // Initialize the result to the original image
  Helpers::DeepCopy<FloatVectorImageType>(image, this->CurrentOutputImage);

  RGBImageType::Pointer rgbImage = RGBImageType::New();
  Helpers::VectorImageToRGBImage(this->CurrentOutputImage, rgbImage);

  Helpers::RGBImageToCIELabImage(rgbImage, this->CIELabImage);
  HelpersOutput::WriteImageConditional<FloatVectorImageType>(this->CIELabImage, "Debug/SetImage.CIELab.mha", this->DebugImages);

  this->FullImageRegion = image->GetLargestPossibleRegion();
  
  ComputeMaxPixelDifference();
}

void CriminisiInpainting::SetMask(const Mask::Pointer mask)
{
  // Initialize the CurrentMask to the OriginalMask
  //Helpers::DeepCopy<Mask>(mask, this->CurrentMask);
  this->CurrentMask->DeepCopyFrom(mask);

  // Save the OriginalMask.
  //Helpers::DeepCopy<Mask>(mask, this->OriginalMask);
  this->OriginalMask->DeepCopyFrom(mask);
}

void CriminisiInpainting::SetDebugMessages(const bool flag)
{
  this->DebugMessages = flag;
}

void CriminisiInpainting::SetDebugImages(const bool flag)
{
  this->DebugImages = flag;
}

itk::ImageRegion<2> CriminisiInpainting::GetFullRegion()
{
  return this->FullImageRegion;
}
