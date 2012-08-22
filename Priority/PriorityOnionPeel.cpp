#include "PriorityOnionPeel.h"

PriorityOnionPeel::PriorityOnionPeel(const Mask* const maskImage, const unsigned int patchRadius) :
MaskImage(maskImage), PatchRadius(patchRadius)
{
  this->ConfidenceMapImage = ITKHelpers::FloatScalarImageType::New();
  InitializeConfidenceMap();
}

void PriorityOnionPeel::InitializeConfidenceMap()
{
  this->ConfidenceMapImage->SetRegions(MaskImage->GetLargestPossibleRegion());
  this->ConfidenceMapImage->Allocate();

  itk::ImageRegionIterator<ConfidenceImageType> imageIterator(ConfidenceMapImage,
                                                              ConfidenceMapImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(MaskImage->IsHole(imageIterator.GetIndex()))
      {
      imageIterator.Set(0.0f);
      }
    else if(MaskImage->IsValid(imageIterator.GetIndex()))
      {
      imageIterator.Set(1.0f);
      }

    ++imageIterator;
    }

//   ITKHelpers::WriteImage(ConfidenceMapImage.GetPointer(), "ConfidenceMapInitial.mha");
//   ITKHelpers::WriteScaledScalarImage(ConfidenceMapImage.GetPointer(), "ConfidenceMapInitial.png");
}
