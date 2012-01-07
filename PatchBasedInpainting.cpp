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
#include "Derivatives.h"
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "MaskOperations.h"
#include "PatchDifferencePixelWiseSum.h"
#include "PriorityFactory.h"
#include "PriorityRandom.h"
#include "SelfPatchCompare.h"
#include "Types.h"

// STL
#include <iostream>

// Boost
#include <boost/bind.hpp>

// ITK
#include "itkBinaryContourImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkGradientImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"
#include "itkVectorImageToImageAdaptor.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

PatchBasedInpainting::PatchBasedInpainting(const FloatVectorImageType* const image, const Mask* const mask)
{
  EnterFunction("PatchBasedInpainting()");

  this->PatchRadius.Fill(3);

  // We don't want to modify the input images, so we copy them.
  this->MaskImage = Mask::New();
  this->MaskImage->DeepCopyFrom(mask);

  this->CurrentInpaintedImage = FloatVectorImageType::New();
  ITKHelpers::DeepCopy<FloatVectorImageType>(image, this->CurrentInpaintedImage);

  ColorImageInsideHole();

  this->FullImageRegion = image->GetLargestPossibleRegion();
  if(this->MaskImage->GetLargestPossibleRegion() != this->FullImageRegion)
    {
    std::cerr << "Mask and image size must match! Mask is " << this->MaskImage->GetLargestPossibleRegion().GetSize()
              << " while image is " << this->FullImageRegion << std::endl;
    exit(-1);
    }

  // We definitely want to update the image and the mask. We can add more images to the list to update later if necessary.
  ImagesToUpdate.push_back(this->CurrentInpaintedImage);
  ImagesToUpdate.push_back(this->MaskImage); // We MUST update the mask LAST, because it is used to know where to update everything else!

  // Set the image to use for pixel to pixel comparisons.
  this->CompareImage = this->CurrentInpaintedImage;

  // Set defaults
  this->NumberOfCompletedIterations = 0;

  this->PriorityFunction = NULL; // Can't initialize this here, must wait until the image and mask are opened
}


FloatVectorImageType* PatchBasedInpainting::GetCurrentOutputImage()
{
  return this->CurrentInpaintedImage;
}

Mask* PatchBasedInpainting::GetMaskImage()
{
  return this->MaskImage;
}

void PatchBasedInpainting::SetPatchRadius(const unsigned int radius)
{
  // Since this is the radius of the patch, there are no restrictions for the radius to be odd or even.
  this->PatchRadius.Fill(radius);
}

unsigned int PatchBasedInpainting::GetPatchRadius() const
{
  return this->PatchRadius[0];
}

const itk::ImageRegion<2>& PatchBasedInpainting::GetFullRegion() const
{
  return this->FullImageRegion;
}

// SelfPatchCompare* PatchBasedInpainting::GetPatchCompare() const
// {
//   return this->PatchCompare.get();
// }

// void PatchBasedInpainting::SetPatchCompare(SelfPatchCompare* patchCompare)
// {
//   delete this->PatchCompare;
//   this->PatchCompare = patchCompare;
// }

void PatchBasedInpainting::SetPriorityFunction(const std::string& priorityName)
{
  this->PriorityFunction = std::shared_ptr<Priority>(PriorityFactory::Create(PriorityFactory::PriorityTypeFromName(priorityName),
                                                                             this->CompareImage, this->MaskImage.GetPointer(), this->PatchRadius[0]));
}

void PatchBasedInpainting::ColorImageInsideHole()
{
  // Color the target image bright green inside the hole. This is helpful when watching the inpainting proceed, as you can clearly see
  // the region that is being filled.

  FloatVectorImageType::PixelType fillColor;
  fillColor.SetSize(this->CurrentInpaintedImage->GetNumberOfComponentsPerPixel());
  fillColor.Fill(0);
  fillColor[0] = 255;
  // We could use MaskImage->ApplyColorToImage here to use a predefined QColor, but this would introduce a dependency on Qt in the non-GUI part of the code.
  this->MaskImage->ApplyToImage<FloatVectorImageType>(this->CurrentInpaintedImage, fillColor);
}

void PatchBasedInpainting::Initialize()
{
  EnterFunction("PatchBasedInpainting::Initialize()");
  try
  {
    // If the user hasn't specified a priority function, use the simplest one.
    if(!this->PriorityFunction)
      {
      std::cout << "Using default Priority function." << std::endl;
      this->PriorityFunction = std::make_shared<PriorityRandom>(this->CurrentInpaintedImage, this->MaskImage, this->PatchRadius[0]);
      }

    this->NumberOfCompletedIterations = 0;

    HelpersOutput::WriteImageConditional<FloatVectorImageType>(this->CurrentInpaintedImage, "Debug/Initialize.CurrentOutputImage.mha", this->DebugImages);

    DebugMessage("Computing source patches...");

    this->SourcePatches = new SourcePatchCollection(this->MaskImage, this->PatchRadius[0]);

    // Clear the source patches, as additional patches are added each iteration. When we reset the inpainter, we want to start over from only patches that are
    // valid in the original mask.
    this->SourcePatches->Clear();

    LeaveFunction("PatchBasedInpainting::Initialize()");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Initialize()!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

PatchPair PatchBasedInpainting::Iterate()
{
  EnterFunction("Iterate()");

  this->PriorityFunction->ComputeAllPriorities();

  SourcePatchCollection::PatchContainer sourcePatches = this->SourcePatches->FindSourcePatchesInRegion(this->FullImageRegion);
  this->SourcePatches->AddPatches(sourcePatches);

  PatchPair usedPatchPair = FindBestPatch();

  std::cout << "Used target region: " << usedPatchPair.GetTargetPatch().GetRegion() << std::endl;

  // Copy the patch. This is the actual inpainting step.
  ImagesToUpdate.CopySelfPatchIntoHoleOfTargetRegion(this->MaskImage, usedPatchPair.GetSourcePatch()->GetRegion(), usedPatchPair.GetTargetPatch().GetRegion());
  std::cout << "Image size: " << this->CurrentInpaintedImage->GetLargestPossibleRegion().GetSize() << std::endl;

  this->PriorityFunction->Update(usedPatchPair.GetTargetPatch().GetRegion());

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(this->PriorityFunction->GetBoundaryImage(), "Debug/BoundaryImageBeforeUpdate.mha");
  this->PriorityFunction->UpdateBoundary();
  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(this->PriorityFunction->GetBoundaryImage(), "Debug/BoundaryImageAfterUpdate.mha");

  // Sanity check everything
  if(this->DebugImages)
    {
    DebugWriteAllImages();
    }

  // Shift the top left corner to a position where the same size patch would overlap only the top left pixel.
  itk::Index<2> previousInvalidRegionIndex;
  previousInvalidRegionIndex[0] = usedPatchPair.GetTargetPatch().GetRegion().GetIndex()[0] - this->PatchRadius[0];
  previousInvalidRegionIndex[1] = usedPatchPair.GetTargetPatch().GetRegion().GetIndex()[1] - this->PatchRadius[1];

  // The region from which patches overlap the used target patch has a radius 2x bigger than the original patch.
  // The computation could be written as (2 * this->PatchRadius[0]) * 2 + 1, or simply this->PatchRadius[0] * 4 + 1
//   itk::Size<2> previousInvalidRegionSize;
//   previousInvalidRegionSize[0] = this->PatchRadius[0] * 4 + 1;
//   previousInvalidRegionSize[1] = this->PatchRadius[1] * 4 + 1;
// 
//   itk::ImageRegion<2> previousInvalidRegion(previousInvalidRegionIndex, previousInvalidRegionSize);

  this->NumberOfCompletedIterations++;

  DebugMessage<unsigned int>("Completed iteration: ", this->NumberOfCompletedIterations);

  LeaveFunction("Iterate()");
  return usedPatchPair;
}

Priority* PatchBasedInpainting::GetPriorityFunction()
{
  return this->PriorityFunction.get();
}

void PatchBasedInpainting::ComputeScores(CandidatePairs& candidatePairs)
{
  //std::cout << "FindBestPatch: There are " << candidatePairs.size() << " candidate pairs." << std::endl;
  this->PatchCompare.SetPairs(&candidatePairs);
  this->PatchCompare.SetImage(this->CompareImage);
  this->PatchCompare.SetMask(this->MaskImage);
  this->PatchCompare.AddDifferenceType(PairDifferences::AveragePixelDifference);
  this->PatchCompare.Compute<PatchDifferencePixelWiseSum<FloatVectorImageType, PixelDifference> >();
}

PatchPair PatchBasedInpainting::FindBestPatch()
{
  EnterFunction("PatchBasedInpainting::FindBestPatch()");

  float highestPriority = 0.0f;
  itk::Index<2> pixelToFill = MaskOperations::FindHighestValueInNonZeroRegion(this->PriorityFunction->GetPriorityImage(),
                                                                      highestPriority, this->PriorityFunction->GetBoundaryImage());

  itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
  Patch targetPatch(targetRegion);
  CandidatePairs candidatePairs(targetPatch);
  candidatePairs.AddSourcePatches(*(this->SourcePatches));
  candidatePairs.SetPriority(highestPriority);

  ComputeScores(candidatePairs);

  candidatePairs.Sort(PairDifferences::AveragePixelDifference);

  //std::cout << "Finished sorting " << candidatePairs.size() << " patches." << std::endl;

  PatchPair bestPatchPair = *(candidatePairs.begin());

  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatch()." << std::endl;
  //LeaveFunction("PatchBasedInpainting::FindBestPatch()");
  return bestPatchPair;
}

void PatchBasedInpainting::Inpaint()
{
  EnterFunction("Inpaint()");
  // This function is intended to be used by the command line version. It will do the complete inpainting without updating any UI or the ability to stop before it is complete.
  try
  {
    // Start the procedure
    //Initialize();

    this->NumberOfCompletedIterations = 0;
    while(HasMoreToInpaint())
      {
      Iterate();
      }
    //std::cout << "Finished inpainting." << std::endl;
    LeaveFunction("Inpaint()");
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Inpaint()!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


bool PatchBasedInpainting::HasMoreToInpaint()
{
  EnterFunction("HasMoreToInpaint()");
  try
  {
    HelpersOutput::WriteImageConditional<Mask>(this->MaskImage, "Debug/HasMoreToInpaint.input.png", this->DebugImages);

    itk::ImageRegionIterator<Mask> maskIterator(this->MaskImage, this->MaskImage->GetLargestPossibleRegion());

    while(!maskIterator.IsAtEnd())
      {
      if(this->MaskImage->IsHole(maskIterator.GetIndex()))
        {
        return true;
        }

      ++maskIterator;
      }

    LeaveFunction("HasMoreToInpaint()");
    // If no pixels were holes, then we don't have any more to inpaint.
    return false;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in HasMoreToInpaint!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

bool PatchBasedInpainting::IsValidPatch(const itk::Index<2>& queryPixel, const unsigned int radius)
{
  // This function checks if a patch is completely inside the image and not intersecting the mask

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, radius);
  return IsValidRegion(region);
}

bool PatchBasedInpainting::IsValidRegion(const itk::ImageRegion<2>& region)
{
  return this->MaskImage->IsValid(region);
}

unsigned int PatchBasedInpainting::GetNumberOfPixelsInPatch()
{
  return this->GetPatchSize()[0]*this->GetPatchSize()[1];
}

itk::Size<2> PatchBasedInpainting::GetPatchSize()
{
  itk::Size<2> patchSize;

  patchSize[0] = Helpers::SideLengthFromRadius(this->PatchRadius[0]);
  patchSize[1] = Helpers::SideLengthFromRadius(this->PatchRadius[1]);

  return patchSize;
}

unsigned int PatchBasedInpainting::GetNumberOfCompletedIterations()
{
  return this->NumberOfCompletedIterations;
}

ITKImageCollection& PatchBasedInpainting::GetImagesToUpdate()
{
  return this->ImagesToUpdate;
}
