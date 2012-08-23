#include "PriorityOnionPeel.h"

PriorityOnionPeel::PriorityOnionPeel(const Mask* const maskImage, const unsigned int patchRadius) :
MaskImage(maskImage), PatchRadius(patchRadius)
{
  this->ConfidenceMapImage = ITKHelpers::FloatScalarImageType::New();
  InitializeConfidenceMap();
}

void PriorityOnionPeel::InitializeConfidenceMap()
{
  this->ConfidenceMapImage->SetRegions(this->MaskImage->GetLargestPossibleRegion());
  this->ConfidenceMapImage->Allocate();

  itk::ImageRegionIterator<ConfidenceImageType> imageIterator(this->ConfidenceMapImage,
                                                              this->ConfidenceMapImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(this->MaskImage->IsHole(imageIterator.GetIndex()))
      {
      imageIterator.Set(0.0f);
      }
    else if(this->MaskImage->IsValid(imageIterator.GetIndex()))
      {
      imageIterator.Set(1.0f);
      }

    ++imageIterator;
    }

//   ITKHelpers::WriteImage(ConfidenceMapImage.GetPointer(), "ConfidenceMapInitial.mha");
//   ITKHelpers::WriteScaledScalarImage(ConfidenceMapImage.GetPointer(), "ConfidenceMapInitial.png");
}
