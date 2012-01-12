
#include "Priority.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"

// VTK
#include <vtkSmartPointer.h>

template <typename TImage>
Priority<TImage>::Priority(const TImage* const image, const Mask* const maskImage, const unsigned int patchRadius) :
                   Image(image), MaskImage(maskImage), PatchRadius(patchRadius)
{
  //std::cout << "Priority() image size: " << image->GetLargestPossibleRegion().GetSize() << std::endl;

  EnterFunction("Priority()");
  this->PriorityImage = FloatScalarImageType::New();
  ITKHelpers::InitializeImage<FloatScalarImageType>(this->PriorityImage, image->GetLargestPossibleRegion());
  ITKHelpers::SetImageToConstant<FloatScalarImageType>(this->PriorityImage, 0.0f);

  this->BoundaryImage = UnsignedCharScalarImageType::New();
  ITKHelpers::InitializeImage<UnsignedCharScalarImageType>(this->BoundaryImage, image->GetLargestPossibleRegion());
  ITKHelpers::SetImageToConstant<UnsignedCharScalarImageType>(this->BoundaryImage, 0u);
  LeaveFunction("Priority()");
}

template <typename TImage>
std::vector<NamedVTKImage> Priority<TImage>::GetNamedImages()
{
  std::vector<NamedVTKImage> namedImages;

  NamedVTKImage priorityImage;
  priorityImage.Name = "Priority";
  vtkSmartPointer<vtkImageData> priorityImageVTK = vtkSmartPointer<vtkImageData>::New();
  ITKVTKHelpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->PriorityImage, priorityImageVTK);
  priorityImage.ImageData = priorityImageVTK;

  namedImages.push_back(priorityImage);

  return namedImages;
}

template <typename TImage>
std::vector<std::string> Priority<TImage>::GetImageNames()
{
  std::vector<std::string> imageNames;
  imageNames.push_back("Priority");
  return imageNames;
}

template <typename TImage>
void Priority<TImage>::Update(const itk::ImageRegion<2>& filledRegion)
{
  ITKHelpers::SetRegionToConstant<FloatScalarImageType>(this->PriorityImage, filledRegion, 0.0f);
  ITKHelpers::SetRegionToConstant<UnsignedCharScalarImageType>(this->BoundaryImage, filledRegion, 0u);
}

template <typename TImage>
FloatScalarImageType* Priority<TImage>::GetPriorityImage()
{
  return this->PriorityImage;
}

template <typename TImage>
UnsignedCharScalarImageType* Priority<TImage>::GetBoundaryImage()
{
  return this->BoundaryImage;
}

template <typename TImage>
float Priority<TImage>::GetPriority(const itk::Index<2>& queryPixel)
{
  return this->PriorityImage->GetPixel(queryPixel);
}

template <typename TImage>
void Priority<TImage>::UpdateBoundary()
{
  EnterFunction("UpdateBoundary()");
  this->MaskImage->FindBoundary(this->BoundaryImage);
  LeaveFunction("UpdateBoundary()");
}

template <typename TImage>
void Priority<TImage>::ComputeAllPriorities()
{
  EnterFunction("ComputeAllPriorities()");

  if(this->MaskImage->GetLargestPossibleRegion() != this->PriorityImage->GetLargestPossibleRegion())
    {
    throw std::runtime_error("Priority::ComputeAllPriorities: The priority image has not been properly initialized!");
    }

  this->MaskImage->FindBoundary(this->BoundaryImage);

  ITKHelpers::SetImageToConstant<FloatScalarImageType>(this->PriorityImage, 0.0f);

  std::vector<itk::Index<2> > boundaryPixels = ITKHelpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage);
  //std::cout << "There are " << boundaryPixels.size() << " boundaryPixels." << std::endl;
  for(unsigned int pixelId = 0; pixelId < boundaryPixels.size(); ++pixelId)
    {
    this->PriorityImage->SetPixel(boundaryPixels[pixelId], ComputePriority(boundaryPixels[pixelId]));
    }

  //std::cout << "Priority image size: " << this->PriorityImage->GetLargestPossibleRegion() << std::endl;

  //HelpersOutput::WriteImage<FloatScalarImageType>(this->PriorityImage, "Debug/Priority.mha");

  LeaveFunction("ComputeAllPriorities()");
}
