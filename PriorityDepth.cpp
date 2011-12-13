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
#include "HelpersOutput.h"

PriorityDepth::PriorityDepth(const FloatVectorImageType* image, const Mask* maskImage, unsigned int patchRadius) : Priority(image, maskImage, patchRadius)
{
  if(!image->GetNumberOfComponentsPerPixel() >= 4)
    {
    std::cerr << "Cannot instantiate PriorityDepth with an image with less than 4 channels!" << std::endl;
    exit(-1);
    }
  
  this->DepthIsophoteImage = FloatVector2ImageType::New();
  Helpers::InitializeImage<FloatVector2ImageType>(this->DepthIsophoteImage, image->GetLargestPossibleRegion());

  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(3);
  indexSelectionFilter->SetInput(image);
  indexSelectionFilter->Update();
  
  HelpersOutput::WriteImage<FloatScalarImageType>(indexSelectionFilter->GetOutput(), "Debug/Depth.mha");
  
  this->BlurredDepth = FloatScalarImageType::New();
  //Helpers::MaskedBlur<FloatScalarImageType>(indexSelectionFilter->GetOutput(), maskImage, 2.0f, this->BlurredDepth);
  
  typedef itk::BilateralImageFilter<FloatScalarImageType, FloatScalarImageType>  BilateralImageFilterType;
  BilateralImageFilterType::Pointer bilateralImageFilter = BilateralImageFilterType::New();
  bilateralImageFilter->SetInput(indexSelectionFilter->GetOutput());
  float domainSigma = 10.0f;
  bilateralImageFilter->SetDomainSigma(domainSigma);
  float rangeSigma = 10.0f;
  bilateralImageFilter->SetRangeSigma(rangeSigma);
  bilateralImageFilter->Update();

  Helpers::DeepCopy<FloatScalarImageType>(bilateralImageFilter->GetOutput(), this->BlurredDepth);
  
  HelpersOutput::WriteImage<FloatScalarImageType>(this->BlurredDepth, "Debug/BlurredDepth.mha");
  
  Derivatives::ComputeMaskedIsophotesInRegion(this->BlurredDepth, maskImage, image->GetLargestPossibleRegion(), this->DepthIsophoteImage);
  HelpersOutput::Write2DVectorImage(this->DepthIsophoteImage, "Debug/BlurredDepthIsophoteImage.mha");

}

// float PriorityDepth::ComputePriority(const itk::Index<2>& queryPixel)
// {
//   float priority = this->DepthIsophoteImage->GetPixel(queryPixel).GetNorm();
//   return priority;
// }

float PriorityDepth::ComputePriority(const itk::Index<2>& queryPixel)
{
  FloatVector2Type isophote = this->DepthIsophoteImage->GetPixel(queryPixel);
  isophote.Normalize();
  
  itk::Index<2> pixelAcrossHole = this->MaskImage->FindPixelAcrossHole(queryPixel, isophote);
  
  FloatVector2Type acrossIsophote = this->DepthIsophoteImage->GetPixel(pixelAcrossHole);
  
  //float priority = Helpers::AngleBetween(isophote, acrossIsophote) * isophote.GetNorm() * acrossIsophote.GetNorm();
  float priority = isophote * acrossIsophote; // dot product
  if(priority < 0.0f)
    {
    acrossIsophote *= -1.0f;
    priority = isophote * acrossIsophote; // dot product
    }

  return priority;
}

void PriorityDepth::Update(const itk::ImageRegion<2>& filledRegion)
{
  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetIndex(3);
  indexSelectionFilter->SetInput(this->Image);
  indexSelectionFilter->Update();

  Helpers::MaskedBlur<FloatScalarImageType>(indexSelectionFilter->GetOutput(), this->MaskImage, 2.0f, this->BlurredDepth);
  Derivatives::ComputeMaskedIsophotesInRegion(indexSelectionFilter->GetOutput(), this->MaskImage, filledRegion, this->DepthIsophoteImage);
}
