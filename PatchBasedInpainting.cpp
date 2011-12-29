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
#include "Helpers.h"
#include "HelpersOutput.h"
#include "Histograms.h"
#include "PatchSorting.h"
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

PatchBasedInpainting::PatchBasedInpainting(const FloatVectorImageType* image, const Mask* mask)
{
  EnterFunction("CriminisiInpainting()");

  this->PatchRadius.Fill(3);

  // We don't want to modify the input images, so we copy them.
  this->MaskImage = Mask::New();
  this->MaskImage->DeepCopyFrom(mask);

  this->CurrentInpaintedImage = FloatVectorImageType::New();
  Helpers::DeepCopy<FloatVectorImageType>(image, this->CurrentInpaintedImage);

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

  this->PatchSortFunction = std::make_shared<SortByDifference>(PatchPair::AverageAbsoluteDifference, PatchSortFunctor::ASCENDING);

  this->PatchCompare = std::make_shared<SelfPatchCompare>();

  this->PriorityFunction = NULL; // Can't initialize this here, must wait until the image and mask are opened
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

bool PatchBasedInpainting::PatchExists(const itk::ImageRegion<2>& region)
{
  for(unsigned int i = 0; i < this->SourcePatches.size(); ++i)
    {
    if(this->SourcePatches[i].Region == region)
      {
      return true;
      }
    }
  return false;
}

std::vector<Patch> PatchBasedInpainting::AddNewSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid and are not already in the source patch list to the list of source patches.
  // Additionally, return the patches that were added.
  EnterFunction("AddNewSourcePatchesInRegion()");

  std::vector<Patch> newPatches;
  try
  {
    // Clearly we cannot add source patches from regions that are outside the image.
    itk::ImageRegion<2> newRegion = Helpers::CropToRegion(region, this->FullImageRegion);

    itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->CurrentInpaintedImage, newRegion);

    while(!imageIterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = imageIterator.GetIndex();
      itk::ImageRegion<2> currentPatchRegion = Helpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius[0]);

      if(this->MaskImage->GetLargestPossibleRegion().IsInside(currentPatchRegion))
	{
	if(this->MaskImage->IsValid(currentPatchRegion))
	  {
	  if(!PatchExists(currentPatchRegion))
	    {
	    //this->SourcePatches.push_back(Patch(this->OriginalImage, region));
	    this->SourcePatches.push_back(Patch(currentPatchRegion));
	    newPatches.push_back(Patch(currentPatchRegion));
	    //DebugMessage("Added a source patch.");
	    }
	  }
	}

      ++imageIterator;
      }
    DebugMessage<unsigned int>("Number of new patches: ", newPatches.size());
    DebugMessage<unsigned int>("Number of source patches: ", this->SourcePatches.size());

    if(this->SourcePatches.size() == 0)
      {
      std::cerr << "There must be at least 1 source patch!" << std::endl;
      exit(-1);
      }

    LeaveFunction("AddNewSourcePatchesInRegion()");
    return newPatches;
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeSourcePatches!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }

}

void PatchBasedInpainting::AddAllSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid to the list of source patches.

  EnterFunction("AddAllSourcePatchesInRegion()");

  try
  {
    // Clearly we cannot add source patches from regions that are outside the image, so crop the desired region to be inside the image.
    itk::ImageRegion<2> newRegion = Helpers::CropToRegion(region, this->FullImageRegion);

    itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->CurrentInpaintedImage, newRegion);

    while(!imageIterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = imageIterator.GetIndex();
      itk::ImageRegion<2> currentPatchRegion = Helpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius[0]);

      if(this->MaskImage->GetLargestPossibleRegion().IsInside(currentPatchRegion))
        {
        if(this->MaskImage->IsValid(currentPatchRegion))
          {
          this->SourcePatches.push_back(Patch(currentPatchRegion));
          }
        }

      ++imageIterator;
      }

    DebugMessage<unsigned int>("Number of source patches: ", this->SourcePatches.size());
    LeaveFunction("AddAllSourcePatchesInRegion()");
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in AddAllSourcePatchesInRegion!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }

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
    this->PotentialCandidatePairs.clear();

    HelpersOutput::WriteImageConditional<FloatVectorImageType>(this->CurrentInpaintedImage, "Debug/Initialize.CurrentOutputImage.mha", this->DebugImages);

    DebugMessage("Computing source patches...");

    // Clear the source patches, as additional patches are added each iteration. When we reset the inpainter, we want to start over from only patches that are
    // valid in the original mask.
    this->SourcePatches.clear();
    AddAllSourcePatchesInRegion(this->FullImageRegion);

    this->PatchCompare->SetNumberOfComponentsPerPixel(this->CompareImage->GetNumberOfComponentsPerPixel());

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

  PriorityFunction->ComputeAllPriorities();

  PatchPair usedPatchPair;

  FindBestPatch(usedPatchPair);

  std::cout << "Used target region: " << usedPatchPair.TargetPatch.Region << std::endl;

  // Copy the patch. This is the actual inpainting step.
  ImagesToUpdate.CopySelfPatchIntoHoleOfTargetRegion(this->MaskImage, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  std::cout << "Image size: " << this->CurrentInpaintedImage->GetLargestPossibleRegion().GetSize() << std::endl;

  this->PriorityFunction->Update(usedPatchPair.TargetPatch.Region);

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
  previousInvalidRegionIndex[0] = usedPatchPair.TargetPatch.Region.GetIndex()[0] - this->PatchRadius[0];
  previousInvalidRegionIndex[1] = usedPatchPair.TargetPatch.Region.GetIndex()[1] - this->PatchRadius[1];

  // The region from which patches overlap the used target patch has a radius 2x bigger than the original patch.
  // The computation could be written as (2 * this->PatchRadius[0]) * 2 + 1, or simply this->PatchRadius[0] * 4 + 1
  itk::Size<2> previousInvalidRegionSize;
  previousInvalidRegionSize[0] = this->PatchRadius[0] * 4 + 1;
  previousInvalidRegionSize[1] = this->PatchRadius[1] * 4 + 1;

  itk::ImageRegion<2> previousInvalidRegion(previousInvalidRegionIndex, previousInvalidRegionSize);

  std::vector<Patch> newPatches = AddNewSourcePatchesInRegion(previousInvalidRegion);

  RecomputeScoresWithNewPatches(newPatches, usedPatchPair);

  this->NumberOfCompletedIterations++;

  DebugMessage<unsigned int>("Completed iteration: ", this->NumberOfCompletedIterations);

  PreviousIterationUsedPatchPair = usedPatchPair;

  LeaveFunction("Iterate()");
  return usedPatchPair;
}

void PatchBasedInpainting::RecomputeScoresWithNewPatches(std::vector<Patch>& newPatches, PatchPair& usedPatchPair)
{
  EnterFunction("RecomputeScoresWithNewPatches()");

  if(newPatches.size() <= 0)
    {
    //std::cout << "There were 0 new patches to recompute!" << std::endl;
    return;
    }
  // Recompute for all forward look candidates except the one that was used. Otherwise there would be an exact match!
  for(unsigned int candidateId = 0; candidateId < this->PotentialCandidatePairs.size(); ++candidateId)
    {
    // Don't bother recomputing for the target patch that was used - it will not be used again.
    if(usedPatchPair.TargetPatch.Region == this->PotentialCandidatePairs[candidateId].TargetPatch.Region)
      {
      continue;
      }
    CandidatePairs newPairs(this->PotentialCandidatePairs[candidateId].TargetPatch);
    newPairs.AddPairsFromPatches(newPatches);

    this->PatchCompare->SetPairs(&newPairs);
    this->PatchCompare->SetImage(this->CompareImage);
    this->PatchCompare->SetMask(this->MaskImage);
    //this->PatchCompare->SetMembershipImage(this->MembershipImage);
    this->PatchCompare->ComputeAllSourceDifferences();

    this->PotentialCandidatePairs[candidateId].Combine(newPairs);
    }

  LeaveFunction("RecomputeScoresWithNewPatches()");
}

Priority* PatchBasedInpainting::GetPriorityFunction()
{
  return this->PriorityFunction.get();
}

void PatchBasedInpainting::FindBestPatch(PatchPair& bestPatchPair)
{
  EnterFunction("PatchBasedInpainting::FindBestPatch()");

  float highestPriority = 0.0f;
  itk::Index<2> pixelToFill = Helpers::FindHighestValueInMaskedRegion(this->PriorityFunction->GetPriorityImage(),
                                                                      highestPriority, this->PriorityFunction->GetBoundaryImage());

  itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
  Patch targetPatch(targetRegion);

  CandidatePairs candidatePairs(targetPatch);
  candidatePairs.AddPairsFromPatches(this->SourcePatches);
  candidatePairs.Priority = highestPriority;

  //std::cout << "FindBestPatch: There are " << candidatePairs.size() << " candidate pairs." << std::endl;
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(this->CompareImage);
  this->PatchCompare->SetMask(this->MaskImage);
  //this->PatchCompare->SetMembershipImage(this->MembershipImage);
  this->PatchCompare->ComputeAllSourceDifferences();

  std::sort(candidatePairs.begin(), candidatePairs.end(), SortFunctorWrapper(this->PatchSortFunction.get()));

  //std::cout << "Finished sorting " << candidatePairs.size() << " patches." << std::endl;

  // Return the result by reference.
  bestPatchPair = candidatePairs[0];

  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatch()." << std::endl;
  LeaveFunction("PatchBasedInpainting::FindBestPatch()");
}

unsigned int PatchBasedInpainting::ComputeMinimumScoreLookAhead()
{
  EnterFunction("ComputeMinimumScoreLookAhead()");
  // Choose the look ahead with the lowest score to actually fill rather than simply returning the best source patch of the first look ahead target patch.
  float lowestScore = std::numeric_limits< float >::max();
  unsigned int lowestLookAhead = 0;
  for(unsigned int i = 0; i < this->PotentialCandidatePairs.size(); ++i)
    {
    if(this->PotentialCandidatePairs[i][0].DifferenceMap[PatchPair::AverageAbsoluteDifference] < lowestScore)
      {
      lowestScore = this->PotentialCandidatePairs[i][0].DifferenceMap[PatchPair::AverageAbsoluteDifference];
      lowestLookAhead = i;
      }
    }
  LeaveFunction("ComputeMinimumScoreLookAhead()");
  return lowestLookAhead;
}

void PatchBasedInpainting::Inpaint()
{
  EnterFunction("Inpaint()");
  // This function is intended to be used by the command line version. It will do the complete inpainting without updating any UI or the ability to stop before it is complete.
  //std::cout << "CriminisiInpainting::Inpaint()" << std::endl;
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

  itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(queryPixel, radius);
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

CandidatePairs& PatchBasedInpainting::GetPotentialCandidatePairReference(const unsigned int forwardLookId)
{
  return this->PotentialCandidatePairs[forwardLookId];
}

std::vector<CandidatePairs> PatchBasedInpainting::GetPotentialCandidatePairs()
{
  return this->PotentialCandidatePairs;
}

unsigned int PatchBasedInpainting::GetNumberOfCompletedIterations()
{
  return this->NumberOfCompletedIterations;
}

ITKImageCollection& PatchBasedInpainting::GetImagesToUpdate()
{
  return this->ImagesToUpdate;
}
