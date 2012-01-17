#include "PriorityOnionPeel.h"

#include "Mask.h"

// ITK
#include "itkInvertIntensityImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

void PriorityOnionPeel::Update(const itk::ImageRegion<2>& filledRegion)
{
  //EnterFunction("PriorityOnionPeel::Update()");
  // Get the center pixel (the pixel around which the region was filled)
//   itk::Index<2> centerPixel = ITKHelpers::GetRegionCenter(filledRegion);
//   float value = ComputeConfidenceTerm(centerPixel);
//   UpdateConfidences(filledRegion, value);

}

float PriorityOnionPeel::ComputePriority(const itk::Index<2>& queryPixel) const
{
//   // Allow for regions on/near the image border
// 
//   //itk::ImageRegion<2> region = this->CurrentMask->GetLargestPossibleRegion();
//   //region.Crop(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
//   itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);
// 
//   // Ensure that the patch to use to compute the confidence is entirely inside the image
//   itk::ImageRegion<2> region = ITKHelpers::CropToRegion(targetRegion, this->MaskImage->GetLargestPossibleRegion());
// 
//   itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);
// 
//   // The confidence is computed as the sum of the confidences of patch pixels in the source region / area of the patch
// 
//   float sum = 0.0f;
// 
//   while(!maskIterator.IsAtEnd())
//     {
//     if(this->MaskImage->IsValid(maskIterator.GetIndex()))
//       {
//       sum += this->ConfidenceMapImage->GetPixel(maskIterator.GetIndex());
//       }
//     ++maskIterator;
//     }
// 
//   unsigned int numberOfPixels = region.GetNumberOfPixels();
//   float areaOfPatch = static_cast<float>(numberOfPixels);
// 
//   float priority = sum/areaOfPatch;
// 
//   return priority;
}

void PriorityOnionPeel::UpdateConfidences(const itk::ImageRegion<2>& targetRegion, const float value)
{
//   // Force the region to update to be entirely inside the image
//   itk::ImageRegion<2> region = ITKHelpers::CropToRegion(targetRegion, this->MaskImage->GetLargestPossibleRegion());
// 
//   // Use an iterator to find masked pixels. Compute their new value, and save it in a vector of pixels and their new values.
//   // Do not update the pixels until after all new values have been computed, because we want to use the old values in all of
//   // the computations.
//   itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);
// 
//   while(!maskIterator.IsAtEnd())
//     {
//     if(this->MaskImage->IsHole(maskIterator.GetIndex()))
//       {
//       itk::Index<2> currentPixel = maskIterator.GetIndex();
//       this->ConfidenceMapImage->SetPixel(currentPixel, value);
//       }
// 
//     ++maskIterator;
//     } // end while looop with iterator

}
