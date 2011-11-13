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

#include "PriorityDepth.h"

#include "Derivatives.h"

PriorityDepth::PriorityDepth(FloatVectorImageType::Pointer image, Mask::Pointer maskImage, unsigned int patchRadius) : Priority(image, maskImage, patchRadius)
{
  this->DepthIsophoteImage = FloatVector2ImageType::New();
  Helpers::InitializeImage<FloatVector2ImageType>(this->DepthIsophoteImage, image->GetLargestPossibleRegion());

  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(3);
  indexSelectionFilter->SetInput(image);
  indexSelectionFilter->Update();

  Derivatives::ComputeMaskedIsophotesInRegion(indexSelectionFilter->GetOutput(), maskImage, image->GetLargestPossibleRegion(), this->DepthIsophoteImage);
}

float PriorityDepth::ComputePriority(const itk::Index<2>& queryPixel)
{
  float priority = this->DepthIsophoteImage->GetPixel(queryPixel).GetNorm();
  return priority;
}

void PriorityDepth::Update(const itk::ImageRegion<2>& filledRegion)
{
  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(3);
  indexSelectionFilter->SetInput(this->Image);
  indexSelectionFilter->Update();

  Derivatives::ComputeMaskedIsophotesInRegion(indexSelectionFilter->GetOutput(), this->MaskImage, filledRegion, this->DepthIsophoteImage);
}
