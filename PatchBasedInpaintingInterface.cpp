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

#include "PatchBasedInpainting.h"

// Custom
#include "ClusterColors.h"
#include "HelpersOutput.h"

// Boost
#include <boost/bind.hpp>

void PatchBasedInpainting::SetPatchSearchFunctionToScaleConsistent()
{
  this->PatchSearchFunction = boost::bind(&PatchBasedInpainting::FindBestPatchScaleConsistent,this,_1,_2);
}

void PatchBasedInpainting::SetPatchSearchFunctionToNormal()
{
  this->PatchSearchFunction = boost::bind(&PatchBasedInpainting::FindBestPatchNormal,this,_1,_2);
}

void PatchBasedInpainting::SetPatchSearchFunctionToTwoStepDepth()
{
  this->PatchSearchFunction = boost::bind(&PatchBasedInpainting::FindBestPatchTwoStepDepth,this,_1,_2);
}

FloatVectorImageType::Pointer PatchBasedInpainting::GetCurrentOutputImage()
{
  return this->CurrentOutputImage;
}
/*
FloatScalarImageType::Pointer PatchBasedInpainting::GetPriorityImage()
{
    return this->PriorityImage;
}

FloatScalarImageType::Pointer PatchBasedInpainting::GetConfidenceImage()
{
  return this->ConfidenceImage;
}

FloatScalarImageType::Pointer PatchBasedInpainting::GetConfidenceMapImage()
{
  return this->ConfidenceMapImage;
}*/

// UnsignedCharScalarImageType::Pointer PatchBasedInpainting::GetBoundaryImage()
// {
//   return this->BoundaryImage;
// }

Mask::Pointer PatchBasedInpainting::GetMaskImage()
{
  return this->MaskImage;
}

// FloatVector2ImageType::Pointer PatchBasedInpainting::GetBoundaryNormalsImage()
// {
//   return this->BoundaryNormals;
// }
// 
// FloatVector2ImageType::Pointer PatchBasedInpainting::GetIsophoteImage()
// {
//   return this->IsophoteImage;
// }

/*
FloatScalarImageType::Pointer PatchBasedInpainting::GetDataImage()
{
  return this->DataImage;
}*/

void PatchBasedInpainting::SetPatchRadius(const unsigned int radius)
{
  // Since this is the radius of the patch, there are no restrictions for the radius to be odd or even.
  this->PatchRadius.Fill(radius);
}

unsigned int PatchBasedInpainting::GetPatchRadius()
{
  return this->PatchRadius[0];
}

void PatchBasedInpainting::SetMaxForwardLookPatches(const unsigned int numberOfPatches)
{
  this->MaxForwardLookPatches = numberOfPatches;
}

void PatchBasedInpainting::SetNumberOfTopPatchesToSave(const unsigned int number)
{
  this->NumberOfTopPatchesToSave = number;
}

void PatchBasedInpainting::SetImage(const FloatVectorImageType::Pointer image)
{
  // Store the original image
  Helpers::DeepCopy<FloatVectorImageType>(image, this->OriginalImage);

  // This must be done here, as CurrentOutputImage must be valid before initializing a priority function.
  Helpers::DeepCopy<FloatVectorImageType>(image, this->CurrentOutputImage);

  RGBImageType::Pointer rgbImage = RGBImageType::New();
  Helpers::VectorImageToRGBImage(image, rgbImage);

  Helpers::RGBImageToCIELabImage(rgbImage, this->CIELabImage);
  HelpersOutput::WriteImageConditional<FloatVectorImageType>(this->CIELabImage, "Debug/SetImage.CIELab.mha", this->DebugImages);

  this->FullImageRegion = image->GetLargestPossibleRegion();

  //this->ColorFrequency.SetDebugFunctionEnterLeave(true);
  
  //unsigned int numberOfBinsPerDimension = 6;
  //this->ColorFrequency.SetNumberOfBinsPerAxis(numberOfBinsPerDimension);

  //unsigned int numberOfColors = 50;
  unsigned int numberOfColors = 100;
  this->ColorFrequency.SetNumberOfColors(numberOfColors);
  this->ColorFrequency.Construct(image);
  
  Helpers::DeepCopy<IntImageType>(this->ColorFrequency.GetColorBinMembershipImage(), this->ColorBinMembershipImage);
  
  //ComputeMaxPixelDifference();
}

void PatchBasedInpainting::SetMask(const Mask::Pointer mask)
{
  // Initialize the CurrentMask to the OriginalMask
  this->MaskImage->DeepCopyFrom(mask);
  //this->MaskImage->SetDebugFunctionEnterLeave(true);
}

itk::ImageRegion<2> PatchBasedInpainting::GetFullRegion()
{
  return this->FullImageRegion;
}

std::vector<CandidatePairs>& PatchBasedInpainting::GetPotentialCandidatePairsReference()
{
  // Return a reference to the whole set of forward look pairs.
  return PotentialCandidatePairs;
}

void PatchBasedInpainting::SetCompareToOriginal()
{
  this->CompareImage = this->CurrentOutputImage;
}
  
void PatchBasedInpainting::SetCompareToBlurred()
{
  this->CompareImage = this->BlurredImage;
}

void PatchBasedInpainting::SetCompareToCIELAB()
{
  this->CompareImage = this->CIELabImage;
}

void PatchBasedInpainting::SetPatchCompare(SelfPatchCompare* patchCompare)
{
  delete this->PatchCompare;
  this->PatchCompare = patchCompare;
}
