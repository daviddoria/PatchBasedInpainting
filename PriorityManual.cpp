#include "PriorityManual.h" // Appease syntax parser

PriorityManual::PriorityManual(const ManualPriorityImageType* manualPriorityImage, Priority* const priorityFunction)
{

}

float PriorityManual::ComputePriority(const itk::Index<2>& queryPixel) const
{
  //std::cout << static_cast<float>(this->ManualPriorityImage->GetPixel(queryPixel)) << std::endl;

  float priority = 0.0f;
  float manualPriority = this->ManualPriorityImage->GetPixel(queryPixel);

  // Make the priority values extremely high, but still sorted.
  float offset = 1e4;
  float normalPriority = this->FallbackPriorityFunction->ComputePriority(queryPixel);
  if(manualPriority > 0)
    {
    priority = offset + normalPriority;
    }
  else
    {
    priority = normalPriority;
    }

  return priority;
}

void PriorityManual::Update(const itk::ImageRegion<2>& filledRegion)
{

}

void PriorityManual::SetManualPriorityImage(ManualPriorityImageType* const image)
{

}
