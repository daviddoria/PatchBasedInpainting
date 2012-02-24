#include "PriorityOnionPeel.h"

PriorityOnionPeel::PriorityOnionPeel(const Mask* const maskImage, unsigned int patchRadius) : MaskImage(maskImage), PatchRadius(patchRadius)
{
  this->ConfidenceMapImage = FloatScalarImageType::New();
  InitializeConfidenceMap();
}

void PriorityOnionPeel::InitializeConfidenceMap()
{
  //EnterFunction("PriorityOnionPeel::InitializeConfidenceMap()");
  // Clone the mask - we need to invert the mask to actually perform the masking, but we don't want to disturb the original mask
  Mask::Pointer maskClone = Mask::New();
  //Helpers::DeepCopy<Mask>(this->CurrentMask, maskClone);
  maskClone->DeepCopyFrom(this->MaskImage);

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <Mask> InvertIntensityImageFilterType;
  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(maskClone);
  invertIntensityFilter->Update();

  // Convert the inverted mask to floats and scale them to between 0 and 1
  // to serve as the initial confidence image
  typedef itk::RescaleIntensityImageFilter< Mask, FloatScalarImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(invertIntensityFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();

  ITKHelpers::DeepCopy<FloatScalarImageType>(rescaleFilter->GetOutput(), this->ConfidenceMapImage);
  //LeaveFunction("PriorityOnionPeel::InitializeConfidenceMap()");
}