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

#include "PriorityOnionPeel.h" // Appease syntax parser

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"
#include "Helpers/OutputHelpers.h"

// ITK
#include "itkInvertIntensityImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// VTK
#include <vtkSmartPointer.h>


template <typename TNode>
void PriorityOnionPeel::Update(const TNode& sourceNode, const TNode& targetNode)
{
  float value = ComputeConfidenceTerm(targetNode);
  UpdateConfidences(targetNode, value);

}

template <typename TNode>
float PriorityOnionPeel::ComputePriority(const TNode& queryPixel) const
{
  float priority = ComputeConfidenceTerm(queryPixel);

  return priority;
}

template <typename TNode>
void PriorityOnionPeel::UpdateConfidences(const TNode& targetNode, const float value)
{
  itk::Index<2> targetPixel = ITKHelpers::CreateIndex(targetNode);

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius);

  // Force the region to update to be entirely inside the image
  region.Crop(this->MaskImage->GetLargestPossibleRegion());

  // Set the hole pixels in the region to 'value'.
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

  while(!maskIterator.IsAtEnd())
    {
    if(this->MaskImage->IsHole(maskIterator.GetIndex()))
      {
      this->ConfidenceMapImage->SetPixel(maskIterator.GetIndex(), value);
      // std::cout << "Set " << maskIterator.GetIndex() << " to " << value << std::endl;
      }

    ++maskIterator;
    } // end while looop with iterator

//   std::cout << "PriorityOnionPeel::UpdateConfidences() writing Confidence Map..." << std::endl;
//   OutputHelpers::WriteImage(ConfidenceMapImage.GetPointer(), "ConfidenceMapUpdated.mha");
//   OutputHelpers::WriteScaledScalarImage(ConfidenceMapImage.GetPointer(), "ConfidenceMapUpdated.png");
}

template <typename TNode>
float PriorityOnionPeel::ComputeConfidenceTerm(const TNode& queryNode) const
{
  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

  // Ensure that the patch to use to compute the confidence is entirely inside the image
  region.Crop(this->MaskImage->GetLargestPossibleRegion());

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

  return confidence;
}
