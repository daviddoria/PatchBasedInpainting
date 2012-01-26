/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "PriorityOnionPeel.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"

// ITK
#include "itkInvertIntensityImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// VTK
#include <vtkSmartPointer.h>

PriorityOnionPeel::PriorityOnionPeel(const Mask* const maskImage, unsigned int patchRadius) : MaskImage(maskImage), PatchRadius(patchRadius)
{
  this->ConfidenceMapImage = FloatScalarImageType::New();
  InitializeConfidenceMap();
}

// template <typename TImage>
// std::vector<std::string> PriorityOnionPeel<TImage>::GetImageNames()
// {
//   std::vector<std::string> imageNames = Priority<TImage>::GetImageNames();
//   imageNames.push_back("ConfidenceMap");
//   return imageNames;
// }
//
// template <typename TImage>
// std::vector<NamedVTKImage> PriorityOnionPeel<TImage>::GetNamedImages()
// {
//   std::vector<NamedVTKImage> namedImages = Priority<TImage>::GetNamedImages();
//
//   NamedVTKImage confidenceMapImage;
//   confidenceMapImage.Name = "ConfidenceMap";
//   vtkSmartPointer<vtkImageData> confidenceMapImageVTK = vtkSmartPointer<vtkImageData>::New();
//   ITKVTKHelpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->ConfidenceMapImage, confidenceMapImageVTK);
//   confidenceMapImage.ImageData = confidenceMapImageVTK;
//   namedImages.push_back(confidenceMapImage);
//
//   return namedImages;
// }

void PriorityOnionPeel::Update(const itk::Index<2>& filledPixel)
{
  //EnterFunction("PriorityOnionPeel::Update()");
  // Get the center pixel (the pixel around which the region was filled)
  float value = ComputeConfidenceTerm(filledPixel);
  UpdateConfidences(filledPixel, value);

}

float PriorityOnionPeel::ComputePriority(const itk::Index<2>& queryPixel) const
{
  float priority = ComputeConfidenceTerm(queryPixel);

  return priority;
}

void PriorityOnionPeel::UpdateConfidences(const itk::Index<2>& targetPixel, const float value)
{
  itk::Size<2> regionSize;
  regionSize.Fill(this->PatchRadius);

  itk::ImageRegion<2> inputRegion(targetPixel, regionSize);
  
  //std::cout << "Updating confidences with value " << value << std::endl;

  // Force the region to update to be entirely inside the image
  itk::ImageRegion<2> region = ITKHelpers::CropToRegion(region, this->MaskImage->GetLargestPossibleRegion());

  // Use an iterator to find masked pixels. Compute their new value, and save it in a vector of pixels and their new values.
  // Do not update the pixels until after all new values have been computed, because we want to use the old values in all of
  // the computations.
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

  while(!maskIterator.IsAtEnd())
    {
    if(this->MaskImage->IsHole(maskIterator.GetIndex()))
      {
      itk::Index<2> currentPixel = maskIterator.GetIndex();
      this->ConfidenceMapImage->SetPixel(currentPixel, value);
      }

    ++maskIterator;
    } // end while looop with iterator
  //LeaveFunction("PriorityOnionPeel::UpdateConfidences()");

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

float PriorityOnionPeel::ComputeConfidenceTerm(const itk::Index<2>& queryPixel) const
{
  //EnterFunction("PriorityOnionPeel::ComputeConfidenceTerm()");
  //DebugMessage<itk::Index<2>>("Computing confidence for ", queryPixel);

  // Allow for regions on/near the image border

  //itk::ImageRegion<2> region = this->CurrentMask->GetLargestPossibleRegion();
  //region.Crop(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
  itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

  // Ensure that the patch to use to compute the confidence is entirely inside the image
  itk::ImageRegion<2> region = ITKHelpers::CropToRegion(targetRegion, this->MaskImage->GetLargestPossibleRegion());

  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

  // The confidence is computed as the sum of the confidences of patch pixels in the source region / area of the patch

  float sum = 0.0f;

  while(!maskIterator.IsAtEnd())
    {
    if(this->MaskImage->IsValid(maskIterator.GetIndex()))
      {
      sum += this->ConfidenceMapImage->GetPixel(maskIterator.GetIndex());
      }
    ++maskIterator;
    }

  unsigned int numberOfPixels = region.GetNumberOfPixels();
  float areaOfPatch = static_cast<float>(numberOfPixels);

  float confidence = sum/areaOfPatch;
  //DebugMessage<float>("Confidence: ", confidence);
  //LeaveFunction("PriorityOnionPeel::ComputeConfidenceTerm()");
  return confidence;

}
