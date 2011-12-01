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

PatchBasedInpainting::PatchBasedInpainting()
{
  EnterFunction("CriminisiInpainting()");
  
  this->PatchRadius.Fill(3);

  this->OriginalImage = FloatVectorImageType::New();
  
  this->MaskImage = Mask::New();
  this->ColorBinMembershipImage = IntImageType::New();
  this->CurrentOutputImage = FloatVectorImageType::New();
  //std::cout << "this->CurrentOutputImage address in constructor: " << this->CurrentOutputImage << std::endl;
  this->CIELabImage = FloatVectorImageType::New();
  this->BlurredImage = FloatVectorImageType::New();
  this->LuminanceImage = FloatScalarImageType::New();

  ImagesToUpdate.AddImage(this->ColorBinMembershipImage);
  ImagesToUpdate.AddImage(this->CurrentOutputImage);
  ImagesToUpdate.AddImage(this->CIELabImage);
  ImagesToUpdate.AddImage(this->BlurredImage);
  ImagesToUpdate.AddImage(this->MaskImage); // We MUST update the mask LAST, because it is used to know where to update everything else!
  //ImagesToUpdate.AddImage(this->LuminanceImage);
  
  // Set the image to use for pixel to pixel comparisons.
  //this->CompareImage = this->CIELabImage;
  //this->CompareImage = this->BlurredImage;
  this->CompareImage = this->CurrentOutputImage;

  this->NumberOfCompletedIterations = 0;
  
  this->HistogramBinsPerDimension = 10;
  this->MaxForwardLookPatches = 10;
  
  this->PatchSortFunction = new SortByDifference(PatchPair::AverageAbsoluteDifference, PatchSortFunctor::ASCENDING);

  //this->PatchSearchFunction = &FindBestPatch;
  this->PatchSearchFunction = boost::bind(&PatchBasedInpainting::FindBestPatchNormal,this,_1,_2);
  
  this->PatchCompare = new SelfPatchCompare;
  this->PatchCompare->ColorFrequency = &(this->ColorFrequency); // Give access to the histograms

  this->PriorityFunction = NULL; // Can't initialize this here, must wait until the image and mask are opened
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

    itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->CurrentOutputImage, newRegion);

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

    itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->CurrentOutputImage, newRegion);

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

void PatchBasedInpainting::InitializeTargetImage()
{
  EnterFunction("InitializeTargetImage()");
  // Initialize to the input.
  std::cout << "InitializeTargetImage: image size: " << this->OriginalImage->GetLargestPossibleRegion().GetSize() << std::endl;
  Helpers::DeepCopy<FloatVectorImageType>(this->OriginalImage, this->CurrentOutputImage);
  std::cout << "InitializeTargetImage: CurrentOutputImage size: " << this->CurrentOutputImage->GetLargestPossibleRegion().GetSize() << std::endl;
  LeaveFunction("InitializeTargetImage()");
}

void PatchBasedInpainting::Initialize()
{
  EnterFunction("PatchBasedInpainting::Initialize()");
  try
  {
    if(this->MaskImage->GetLargestPossibleRegion() != this->OriginalImage->GetLargestPossibleRegion())
      {
      std::cerr << "Original mask size does not match original image size!" << std::endl;
      std::cerr << "Original mask size: " << this->MaskImage->GetLargestPossibleRegion() << std::endl;
      std::cerr << "Original image size: " << this->OriginalImage->GetLargestPossibleRegion() << std::endl;
      exit(-1);
      }
      
    // Initialize the result to the original image
    InitializeTargetImage();

    // If the user hasn't specified a priority function, use the simplest one.
    if(!this->PriorityFunction)
      {
      std::cout << "Using default Priority function." << std::endl;
      this->PriorityFunction = new PriorityRandom(this->CurrentOutputImage, this->MaskImage, this->PatchRadius[0]);
      }

    this->NumberOfCompletedIterations = 0;
    this->PotentialCandidatePairs.clear();

    HelpersOutput::WriteImageConditional<FloatVectorImageType>(this->CurrentOutputImage, "Debug/Initialize.CurrentOutputImage.mha", this->DebugImages);

    // Blur the image incase we want to use a blurred image for pixel to pixel comparisons.
    //unsigned int kernelRadius = 5;
    //Helpers::VectorMaskedBlur(this->OriginalImage, this->CurrentMask, kernelRadius, this->BlurredImage);
    
    // Construct the histogram kdtree and membership image
    //this->ColorFrequency.SetDebugFunctionEnterLeave(true);
    
    //unsigned int numberOfBinsPerDimension = 6;
    //this->ColorFrequency.SetNumberOfBinsPerAxis(numberOfBinsPerDimension);

    this->ColorFrequency.SetNumberOfColors(20);
    //this->ColorFrequency.SetDownsampleFactor(20);
    this->ColorFrequency.Construct(this->CurrentOutputImage);
    
    Helpers::DeepCopy<IntImageType>(this->ColorFrequency.GetColorBinMembershipImage(), this->ColorBinMembershipImage);
    HelpersOutput::WriteImage<IntImageType>(this->ColorBinMembershipImage, "Debug/SetImage.ColorBinMembershipImage.mha");
    
    // If the user hasn't provided a blurred image, blur the image.
    if(this->BlurredImage->GetLargestPossibleRegion().GetSize()[0] == 0)
      {
      BlurImage();
      }

    // Initialize internal images
    //Helpers::InitializeImage<FloatScalarImageType>(this->DataImage, this->FullImageRegion);
    //Helpers::InitializeImage<FloatScalarImageType>(this->PriorityImage, this->FullImageRegion);
    //Helpers::InitializeImage<FloatScalarImageType>(this->ConfidenceImage, this->FullImageRegion);
        
    //InitializeConfidenceMap();
    //HelpersOutput::WriteImageConditional<FloatScalarImageType>(this->ConfidenceMapImage, "Debug/Initialize.ConfidenceMapImage.mha", this->DebugImages);

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

  FindBestPatchLookAhead(usedPatchPair);

  std::cout << "Used target region: " << usedPatchPair.TargetPatch.Region << std::endl;

//   if(this->DebugImages)
//     {
//     std::stringstream ssSource;
//     ssSource << "Debug/source_" << Helpers::ZeroPad(this->NumberOfCompletedIterations, 3) << ".mha";
//     HelpersOutput::WritePatch<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.SourcePatch, ssSource.str());
// 
//     std::stringstream ssTargetMHA;
//     ssTargetMHA << "Debug/target_" << Helpers::ZeroPad(this->NumberOfCompletedIterations, 3) << ".mha";
//     HelpersOutput::WritePatch<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.TargetPatch, ssTargetMHA.str());
// 
//     std::stringstream ssTargetPNG;
//     ssTargetPNG << "Debug/target_" << Helpers::ZeroPad(this->NumberOfCompletedIterations, 3) << ".png";
//     Helpers::WriteRegionUnsignedChar<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.TargetPatch.Region, ssTargetPNG.str());
//     }

  // Copy the patch. This is the actual inpainting step.
  ImagesToUpdate.CopySelfPatchIntoHoleOfTargetRegion(this->MaskImage, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  std::cout << "Image size: " << this->CurrentOutputImage->GetLargestPossibleRegion().GetSize() << std::endl;
  std::cout << "Blurred size: " << this->BlurredImage->GetLargestPossibleRegion().GetSize() << std::endl;
  
  this->PriorityFunction->Update(usedPatchPair.TargetPatch.Region);

  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(this->PriorityFunction->GetBoundaryImage(), "Debug/BoundaryImageBeforeUpdate.mha");
  this->PriorityFunction->UpdateBoundary();
  HelpersOutput::WriteImage<UnsignedCharScalarImageType>(this->PriorityFunction->GetBoundaryImage(), "Debug/BoundaryImageAfterUpdate.mha");

  // Sanity check everything
  if(this->DebugImages)
    {
    DebugWriteAllImages();
    }

  // Add new source patches
  // Get the region of pixels which were previous touching the hole which was the target region.

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

  //std::cout << "Used target region: " << usedPatchPair.TargetPatch.Region << std::endl;
  //std::cout << "Previously invalid region: " << previousInvalidRegion << std::endl;

  std::vector<Patch> newPatches = AddNewSourcePatchesInRegion(previousInvalidRegion);

  RecomputeScoresWithNewPatches(newPatches, usedPatchPair);

  // At the end of an iteration, things would be slightly out of sync. The boundary is computed at the beginning of the iteration (before the patch is filled),
  // so at the end of the iteration, the boundary image will not correspond to the boundary of the remaining hole in the image - it will be off by 1 iteration.
  // We fix this by computing the boundary and boundary normals again at the end of the iteration.
  //FindBoundary();
  //ComputeBoundaryNormals(blurVariance);

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
    this->PatchCompare->SetMembershipImage(this->ColorBinMembershipImage);
    this->PatchCompare->ComputeAllSourceDifferences();

    this->PotentialCandidatePairs[candidateId].Combine(newPairs);
    }

  LeaveFunction("RecomputeScoresWithNewPatches()");
}

Priority* PatchBasedInpainting::GetPriorityFunction()
{
  return this->PriorityFunction;
}

void PatchBasedInpainting::FindBestPatchScaleConsistent(CandidatePairs& candidatePairs, PatchPair& bestPatchPair)
{
  //std::cout << "FindBestPatchScaleConsistent: There are " << this->SourcePatches.size() << " source patches at the beginning." << std::endl;
  EnterFunction("FindBestPatchScaleConsistent()");
  
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(this->BlurredImage);
  this->PatchCompare->SetMask(this->MaskImage);
  this->PatchCompare->SetMembershipImage(this->ColorBinMembershipImage);
  this->PatchCompare->ComputeAllSourceDifferences();
  
  std::sort(candidatePairs.begin(), candidatePairs.end(), SortFunctorWrapper(this->PatchSortFunction));
  //std::cout << "Blurred score for pair 0: " << candidatePairs[0].GetAverageAbsoluteDifference() << std::endl;
  candidatePairs.InvalidateAll();
  
  std::vector<float> blurredScores(candidatePairs.size());
  for(unsigned int i = 0; i < candidatePairs.size(); ++i)
    {
    blurredScores[i] = candidatePairs[i].DifferenceMap[PatchPair::AverageAbsoluteDifference];
    }

  // Create a temporary image to fill for now, but we might not actually end up filling this patch.
  FloatVectorImageType::Pointer tempImage = FloatVectorImageType::New();
  Helpers::DeepCopy<FloatVectorImageType>(this->CurrentOutputImage, tempImage);
  // Fill the detailed image hole with a part of the blurred image
  Helpers::CopySourcePatchIntoHoleOfTargetRegion<FloatVectorImageType>(this->BlurredImage, tempImage, this->MaskImage, candidatePairs[0].SourcePatch.Region, candidatePairs[0].TargetPatch.Region);

  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(tempImage);
  this->PatchCompare->SetMask(this->MaskImage);
  this->PatchCompare->SetMembershipImage(this->ColorBinMembershipImage);
  this->PatchCompare->ComputeAllSourceAndTargetDifferences();

  //std::cout << "Detailed score for pair 0: " << candidatePairs[0].GetAverageAbsoluteDifference() << std::endl;
  
  for(unsigned int i = 0; i < candidatePairs.size(); ++i)
    {
    candidatePairs[i].DifferenceMap[PatchPair::AverageAbsoluteDifference] = blurredScores[i] + candidatePairs[i].DifferenceMap[PatchPair::AverageAbsoluteDifference];
    }

  std::cout << "Total score for pair 0: " << candidatePairs[0].DifferenceMap[PatchPair::AverageAbsoluteDifference] << std::endl;
  std::sort(candidatePairs.begin(), candidatePairs.end(), SortFunctorWrapper(this->PatchSortFunction));

  // Return the result by reference.
  bestPatchPair = candidatePairs[0];
  
  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatchScaleConsistent()." << std::endl;
  LeaveFunction("FindBestPatchScaleConsistent()");
}


void PatchBasedInpainting::FindBestPatchNormal(CandidatePairs& candidatePairs, PatchPair& bestPatchPair)
{
  EnterFunction("FindBestPatchNormal()");

  //std::cout << "FindBestPatch: There are " << candidatePairs.size() << " candidate pairs." << std::endl;
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(this->CompareImage);
  this->PatchCompare->SetMask(this->MaskImage);
  this->PatchCompare->SetMembershipImage(this->ColorBinMembershipImage);
  this->PatchCompare->ComputeAllSourceDifferences();

  //std::cout << "FindBestPatch: Finished ComputeAllSourceDifferences()" << std::endl;

  //std::sort(candidatePairs.begin(), candidatePairs.end(), SortByAverageAbsoluteDifference);
  //std::sort(candidatePairs.begin(), candidatePairs.end(), SortByDepthAndColor);
  std::sort(candidatePairs.begin(), candidatePairs.end(), SortFunctorWrapper(this->PatchSortFunction));

  //std::cout << "Finished sorting " << candidatePairs.size() << " patches." << std::endl;

  // Return the result by reference.
  bestPatchPair = candidatePairs[0];

  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatch()." << std::endl;
  LeaveFunction("FindBestPatchNormal()");
}

void PatchBasedInpainting::FindBestPatchTwoStepDepth(CandidatePairs& candidatePairs, PatchPair& bestPatchPair)
{
  EnterFunction("FindBestPatchTwoStepDepth()");

  //std::cout << "FindBestPatch: There are " << candidatePairs.size() << " candidate pairs." << std::endl;
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(this->CompareImage);
  this->PatchCompare->SetMask(this->MaskImage);
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetMembershipImage(this->ColorBinMembershipImage);
  
  this->PatchCompare->FunctionsToCompute.clear();
  this->PatchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchDepthDifference,this->PatchCompare,_1));
  this->PatchCompare->ComputeAllSourceDifferences();
  
  //std::cout << "FindBestPatch: Finished ComputeAllSourceDifferences()" << std::endl;
  
  std::sort(candidatePairs.begin(), candidatePairs.end(), SortByDifference(PatchPair::DepthDifference, PatchSortFunctor::ASCENDING));
  
  WriteImageOfScores(candidatePairs, this->CurrentOutputImage->GetLargestPossibleRegion(),
                     Helpers::GetSequentialFileName("Debug/ImageOfDepthScores", this->NumberOfCompletedIterations, "mha"));
  //candidatePairs.WriteDepthScoresToFile("candidateScores.txt");
  
  CandidatePairs goodDepthCandidatePairs;
  goodDepthCandidatePairs.CopyMetaOnly(candidatePairs);
  goodDepthCandidatePairs.insert(goodDepthCandidatePairs.end(), candidatePairs.begin(), candidatePairs.begin() + 1000);
  this->PatchCompare->SetPairs(&goodDepthCandidatePairs);
  
  //goodDepthCandidatePairs.WriteDepthScoresToFile("depthScores.txt");
  
  this->PatchCompare->FunctionsToCompute.clear();
  this->PatchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,this->PatchCompare,_1));
  this->PatchCompare->ComputeAllSourceDifferences();
  
  std::sort(goodDepthCandidatePairs.begin(), goodDepthCandidatePairs.end(), SortByDifference(PatchPair::AverageAbsoluteDifference, PatchSortFunctor::ASCENDING));
  WriteImageOfScores(goodDepthCandidatePairs, this->CurrentOutputImage->GetLargestPossibleRegion(),
                     Helpers::GetSequentialFileName("Debug/ImageOfTopColorScores", this->NumberOfCompletedIterations, "mha"));
  //std::cout << "Finished sorting " << candidatePairs.size() << " patches." << std::endl;
  
  // Return the result by reference.
  bestPatchPair = goodDepthCandidatePairs[0];
  
  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatch()." << std::endl;
  LeaveFunction("FindBestPatchTwoStepDepth()");
}

void PatchBasedInpainting::FindBestPatchLookAhead(PatchPair& bestPatchPair)
{
  EnterFunction("FindBestPatchLookAhead()");
  // This function returns the best PatchPair by reference

  //std::cout << "FindBestPatchLookAhead: There are " << this->SourcePatches.size() << " source patches at the beginning." << std::endl;

  // If this is not the first iteration, get the potential forward look patch candidates from the previous step
  if(this->NumberOfCompletedIterations > 0)
    {
    // Remove the patch candidate that was actually filled
    unsigned int idToRemove = 0;
    for(unsigned int forwardLookId = 0; forwardLookId < this->PotentialCandidatePairs.size(); ++forwardLookId)
      {
      if(PreviousIterationUsedPatchPair.TargetPatch.Region == this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region)
        {
        idToRemove = forwardLookId;
        break;
        }
      }
    this->PotentialCandidatePairs.erase(this->PotentialCandidatePairs.begin() + idToRemove);
    DebugMessage<unsigned int>("Removed forward look: ", idToRemove);
    }

  // We need to temporarily modify the priority image and boundary image without affecting the actual images, so we copy them.
  FloatScalarImageType::Pointer modifiedPriorityImage = FloatScalarImageType::New();
  Helpers::DeepCopy<FloatScalarImageType>(this->PriorityFunction->GetPriorityImage(), modifiedPriorityImage);

  UnsignedCharScalarImageType::Pointer modifiedBoundaryImage = UnsignedCharScalarImageType::New();
  Helpers::DeepCopy<UnsignedCharScalarImageType>(this->PriorityFunction->GetBoundaryImage(), modifiedBoundaryImage);

  // Blank all regions that are already look ahead patches.
  for(unsigned int forwardLookId = 0; forwardLookId < this->PotentialCandidatePairs.size(); ++forwardLookId)
    {
    Helpers::SetRegionToConstant<FloatScalarImageType>(modifiedPriorityImage, this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region, 0.0f);
    Helpers::SetRegionToConstant<UnsignedCharScalarImageType>(modifiedBoundaryImage, this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region, 0);
    }

  // Find the remaining number of patch candidates
  unsigned int numberOfNewPatchesToFind = this->MaxForwardLookPatches - this->PotentialCandidatePairs.size();
  for(unsigned int newPatchId = 0; newPatchId < numberOfNewPatchesToFind; ++newPatchId)
    {
    //std::cout << "FindBestPatchLookAhead: Start computing new patch " << newPatchId << std::endl;

    // If there are no boundary pixels, we can't find any more look ahead patches. This is not an error - near the end of the inpainting this will most likely occur. It is ok - we just don't have to compute as many forward look patches.
    if(Helpers::CountNonZeroPixels<UnsignedCharScalarImageType>(modifiedBoundaryImage) == 0)
      {
      //std::cout << "FindBestPatchLookAhead: There are no more boundary pixels!" << std::endl;
      break;
      }

    float highestPriority = 0.0f;
    itk::Index<2> pixelToFill = Helpers::FindHighestValueInMaskedRegion(modifiedPriorityImage, highestPriority, modifiedBoundaryImage);
    //std::cout << "Highest priority: " << highestPriority << std::endl;
    if(!this->MaskImage->HasHoleNeighbor(pixelToFill))
      {
      std::cerr << "pixelToFill " << pixelToFill << " does not have a hole neighbor - something is wrong!" << std::endl;
      std::cerr << "Mask value " << static_cast<unsigned int>(this->MaskImage->GetPixel(pixelToFill)) << std::endl;
      HelpersOutput::WriteImage<Mask>(this->MaskImage, "Debug/MaskImageError.mha");
      HelpersOutput::WriteImage<UnsignedCharScalarImageType>(modifiedBoundaryImage, "Debug/ModifiedBoundaryImageError.mha");
      HelpersOutput::WriteImage<FloatScalarImageType>(modifiedPriorityImage, "Debug/ModifiedPriorityImageError.mha");
      //std::cerr << "Boundary value " << static_cast<unsigned int>(this->BoundaryImage->GetPixel(pixelToFill)) << std::endl;
      exit(-1);
      }

    itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
    Patch targetPatch(targetRegion);

    CandidatePairs candidatePairs(targetPatch);
    candidatePairs.AddPairsFromPatches(this->SourcePatches);
    candidatePairs.Priority = highestPriority;

    PatchPair currentLookAheadBestPatchPair;
    this->PatchSearchFunction(candidatePairs, currentLookAheadBestPatchPair);

    // Keep only the number of top patches specified.
    //patchPairsSortedByContinuation.erase(patchPairsSortedByContinuation.begin() + this->NumberOfTopPatchesToSave, patchPairsSortedByContinuation.end());

    // Blank a region around the current potential patch to fill. This will ensure the next potential patch to fill is reasonably far away.
    Helpers::SetRegionToConstant<FloatScalarImageType>(modifiedPriorityImage, targetRegion, 0.0f);
    Helpers::SetRegionToConstant<UnsignedCharScalarImageType>(modifiedBoundaryImage, targetRegion, 0);

    //std::cout << "Sorted " << candidatePairs.size() << " candidatePairs." << std::endl;

    this->PotentialCandidatePairs.push_back(candidatePairs);
    //std::cout << "FindBestPatchLookAhead: Finished computing new patch " << newPatchId << std::endl;
    } // end forward look loop

  if(this->PotentialCandidatePairs.size() == 0)
    {
    std::cerr << "Something is wrong - there are 0 forward look candidates!" << std::endl;
    exit(-1);
    }

  // Sort the forward look patches so that the highest priority sets are first in the vector (descending order).
  std::sort(this->PotentialCandidatePairs.rbegin(), this->PotentialCandidatePairs.rend(), SortByPriority);

  unsigned int bestForwardLookId = ComputeMinimumScoreLookAhead();

  unsigned int bestSourcePatchId = 0;
  //unsigned int bestSourcePatchId = GetRequiredHistogramIntersection(bestForwardLookId);

  std::cout << "Best pair found to be " << bestForwardLookId << " " << bestSourcePatchId << std::endl;
    
  // Return the result by reference.
  bestPatchPair = this->PotentialCandidatePairs[bestForwardLookId][bestSourcePatchId];
  
//   std::cout << "Used patch " << sourcePatchId - 1 << std::endl;
  
  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end." << std::endl;
  LeaveFunction("FindBestPatchLookAhead()");
}

unsigned int PatchBasedInpainting::GetRequiredHistogramIntersection(const unsigned int bestForwardLookId)
{
  float histogramIntersection = 0.0f;
  unsigned int sourcePatchId = 0;

  //this->ColorFrequency.SetDebugFunctionEnterLeave(true);
  float requiredHistogramIntersection = 0.75f;
  do
  {
    PatchPair patchPair = this->PotentialCandidatePairs[bestForwardLookId][sourcePatchId];
    //std::vector<float> histogram1 = Histograms::Compute1DHistogramOfMultiChannelMaskedImage(this->CurrentOutputImage, bestPatchPair.TargetPatch.Region, this->MaskImage, bestPatchPair.TargetPatch.Region, 50);
    //std::vector<float> histogram2 = Histograms::Compute1DHistogramOfMultiChannelMaskedImage(this->CurrentOutputImage, bestPatchPair.SourcePatch.Region, inverseMask, bestPatchPair.TargetPatch.Region, 50);
    std::vector<float> histogram1 = this->ColorFrequency.HistogramRegion(this->ColorBinMembershipImage,
                                                                         patchPair.TargetPatch.Region, this->MaskImage, patchPair.TargetPatch.Region);
    std::vector<float> histogram2 = this->ColorFrequency.HistogramRegion(this->ColorBinMembershipImage,
                                                                         patchPair.SourcePatch.Region, this->MaskImage, patchPair.TargetPatch.Region, true);
    sourcePatchId++; // Note at the end of the loop bestPatchPair will have been set to the previous patchId.
    histogramIntersection = Histograms::HistogramIntersection(histogram2, histogram1);
    //std::cout << "histogramIntersection: " << histogramIntersection << std::endl;
    std::stringstream ssSource;
    ssSource << "/home/doriad/Debug/" << this->NumberOfCompletedIterations << "_" << Helpers::ZeroPad(sourcePatchId, 4) << "_source.txt";
    std::stringstream ssTarget;
    ssTarget << "/home/doriad/Debug/" << this->NumberOfCompletedIterations << "_" << Helpers::ZeroPad(sourcePatchId, 4) << "_target.txt";
    //Histograms::WriteHistogram(histogram1, ssSource.str());
    //Histograms::WriteHistogram(histogram2, ssTarget.str());
  } while (histogramIntersection < requiredHistogramIntersection && sourcePatchId < this->PotentialCandidatePairs[bestForwardLookId].size());
  
  return sourcePatchId;
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


void PatchBasedInpainting::UpdateMask(const itk::ImageRegion<2>& region)
{
  EnterFunction("UpdateMask()");
  try
  {
    itk::ImageRegionIterator<Mask> maskIterator(this->MaskImage, region);

    while(!maskIterator.IsAtEnd())
      {
      if(this->MaskImage->IsHole(maskIterator.GetIndex()))
        {
        maskIterator.Set(this->MaskImage->GetValidValue());
        }

      ++maskIterator;
      }
    LeaveFunction("UpdateMask()");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateMask!" << std::endl;
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

void PatchBasedInpainting::BlurImage()
{
  EnterFunction("BlurImage()");
  float blurVariance = 1.0;
  Helpers::AnisotropicBlurAllChannels<FloatVectorImageType>(this->OriginalImage, this->BlurredImage, blurVariance);
  HelpersOutput::WriteImageConditional<FloatVectorImageType>(this->BlurredImage, "Debug/Initialize.BlurredImage.mha", this->DebugImages);
  HelpersOutput::WriteVectorImageAsRGB(this->BlurredImage, "Debug/Initialize.BlurredImageRGB.mha");
  LeaveFunction("BlurImage()");
}
