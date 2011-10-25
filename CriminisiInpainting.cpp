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

#include "CriminisiInpainting.h"

// Custom
#include "Helpers.h"
#include "RotateVectors.h"

#include "SelfPatchCompare.h"
#include "SelfPatchCompareColor.h"
#include "SelfPatchCompareAll.h"

// STL
#include <iostream>

// VXL
#include <vnl/vnl_double_2.h>

// ITK
#include "itkBinaryContourImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkGradientImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"


CriminisiInpainting::CriminisiInpainting()
{
  this->PatchRadius.Fill(3);

  this->BoundaryImage = UnsignedCharScalarImageType::New();
  this->BoundaryNormals = FloatVector2ImageType::New();
  this->IsophoteImage = FloatVector2ImageType::New();
  this->PriorityImage = FloatScalarImageType::New();
  this->OriginalMask = Mask::New();
  this->CurrentMask = Mask::New();
  this->OriginalImage = FloatVectorImageType::New();
  this->CurrentOutputImage = FloatVectorImageType::New();
  this->CIELabImage = FloatVectorImageType::New();
  this->BlurredImage = FloatVectorImageType::New();
  
  // Set the image to use for pixel to pixel comparisons.
  //this->CompareImage = this->CIELabImage;
  this->CompareImage = this->BlurredImage;
  
  this->ConfidenceImage = FloatScalarImageType::New();
  this->ConfidenceMapImage = FloatScalarImageType::New();
  this->DataImage = FloatScalarImageType::New();
  
  this->DebugImages = false;
  this->DebugMessages = false;
  this->NumberOfCompletedIterations = 0;
  
  this->HistogramBinsPerDimension = 10;
  this->MaxForwardLookPatches = 10;
  
  this->MaxPixelDifferenceSquared = 0.0f;
}

void CriminisiInpainting::ComputeMaxPixelDifference()
{
  // We assume all values of all channels are positive, so the max difference can be computed as max(p_i - 0) because (0,0,0,...) is the minimum pixel.
  
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->OriginalImage, this->OriginalImage->GetLargestPossibleRegion());

  FloatVectorImageType::PixelType zeroPixel;
  zeroPixel.SetSize(this->OriginalImage->GetNumberOfComponentsPerPixel());
  zeroPixel.Fill(0);
  
  FloatVectorImageType::PixelType maxPixel = zeroPixel;
  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = this->OriginalImage->GetPixel(imageIterator.GetIndex());
    //std::cout << "pixel: " << pixel << " Norm: " << pixel.GetNorm() << std::endl;
    if(pixel.GetNorm() > maxPixel.GetNorm())
      {
      maxPixel = pixel;
      }
    ++imageIterator;
    }

  /*
  this->MaxPixelDifference = 0.0f;
  for(unsigned int i = 0; i < this->OriginalImage->GetNumberOfComponentsPerPixel(); ++i)
    {
    this->MaxPixelDifference += maxPixel[i];
    }
  */
  this->MaxPixelDifferenceSquared = SelfPatchCompareAll::StaticPixelDifferenceSquared(maxPixel, zeroPixel);
  
  std::cout << "MaxPixelDifference computed to be: " << this->MaxPixelDifferenceSquared << std::endl;
}

std::vector<Patch> CriminisiInpainting::AddSourcePatches(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely Valid to the list of source patches.
  // TODO: This should somehow check to make sure the patch isn't already in the list before adding it.
  
  DebugMessage("AddSourcePatches()");
  
  std::vector<Patch> newPatches;
  try
  {
    itk::ImageRegion<2> newRegion = CropToValidRegion(region);
    //this->SourcePatches.clear();
    itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->CurrentOutputImage, newRegion);

    while(!imageIterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = imageIterator.GetIndex();
      itk::ImageRegion<2> currentPatchRegion = Helpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius[0]);
    
      if(this->CurrentMask->GetLargestPossibleRegion().IsInside(currentPatchRegion))
	{
	if(this->CurrentMask->IsValid(currentPatchRegion))
	  {
	  //this->SourcePatches.push_back(Patch(this->OriginalImage, region));
	  this->SourcePatches.push_back(Patch(currentPatchRegion));
	  newPatches.push_back(Patch(currentPatchRegion));
	  //DebugMessage("Added a source patch.");
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
    return newPatches;
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeSourcePatches!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }

}

void CriminisiInpainting::InitializeConfidenceMap()
{
  DebugMessage("InitializeConfidenceMap()");
  // Clone the mask - we need to invert the mask to actually perform the masking, but we don't want to disturb the original mask
  Mask::Pointer maskClone = Mask::New();
  //Helpers::DeepCopy<Mask>(this->CurrentMask, maskClone);
  maskClone->DeepCopyFrom(this->CurrentMask);
  
  // Invert the mask
  typedef itk::InvertIntensityImageFilter <Mask> InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(maskClone);
  //invertIntensityFilter->InPlaceOn();
  invertIntensityFilter->Update();

  // Convert the inverted mask to floats and scale them to between 0 and 1
  // to serve as the initial confidence image
  typedef itk::RescaleIntensityImageFilter< Mask, FloatScalarImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(invertIntensityFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();

  Helpers::DeepCopy<FloatScalarImageType>(rescaleFilter->GetOutput(), this->ConfidenceMapImage);

}

void CriminisiInpainting::InitializeTargetImage()
{
  DebugMessage("InitializeTargetImage()");
  // Initialize to the input
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(this->OriginalImage, this->CurrentOutputImage);
}

void CriminisiInpainting::Initialize()
{
  DebugMessage("Initialize()");
  try
  {
    this->NumberOfCompletedIterations = 0;
  
    // We find the boundary of the mask at every iteration (in Iterate()), but we do this here so that everything is initialized and valid if we are observing this class for display.
    FindBoundary();
    
    InitializeTargetImage();
    Helpers::DebugWriteImageConditional<FloatVectorImageType>(this->CurrentOutputImage, "Debug/Initialize.CurrentOutputImage.mha", this->DebugImages);
  
    // Compute isophotes
    {
    RGBImageType::Pointer rgbImage = RGBImageType::New();
    Helpers::VectorImageToRGBImage(this->OriginalImage, rgbImage);
    
    Helpers::DebugWriteImageConditional<RGBImageType>(rgbImage, "Debug/Initialize.rgb.mha", this->DebugImages);

    typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
    LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
    luminanceFilter->SetInput(rgbImage);
    luminanceFilter->Update();
    
    FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
    // Blur with a Gaussian kernel. From TestIsophotes.cpp, we have determined that after a blurring radius of 4 the resulting isophotes are indistinguishable.
    unsigned int kernelRadius = 5;
    Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), this->CurrentMask, kernelRadius, blurredLuminance);
    
    Helpers::DebugWriteImageConditional<FloatScalarImageType>(blurredLuminance, "Debug/ComputeMaskedIsophotes.blurred.mha", true);
    
    ComputeMaskedIsophotes(blurredLuminance, this->CurrentMask);
    if(this->DebugImages)
      {
      Helpers::Write2DVectorImage(this->IsophoteImage, "Debug/Initialize.IsophoteImage.mha");
      }
    }
    
    // Blur the image incase we want to use a blurred image for pixel to pixel comparisons.
    unsigned int kernelRadius = 5;
    Helpers::VectorMaskedBlur(this->OriginalImage, this->CurrentMask, kernelRadius, this->BlurredImage);
    Helpers::DebugWriteImageConditional<FloatVectorImageType>(this->BlurredImage, "Debug/Initialize.BlurredImage.mha", this->DebugImages);
    
    Helpers::InitializeImage<FloatScalarImageType>(this->DataImage, this->FullImageRegion);
    
    Helpers::InitializeImage<FloatScalarImageType>(this->PriorityImage, this->FullImageRegion);
    
    Helpers::InitializeImage<FloatScalarImageType>(this->ConfidenceImage, this->FullImageRegion);
        
    InitializeConfidenceMap();
    Helpers::DebugWriteImageConditional<FloatScalarImageType>(this->ConfidenceMapImage, "Debug/Initialize.ConfidenceMapImage.mha", this->DebugImages);

    DebugMessage("Computing source patches...");
    AddSourcePatches(this->FullImageRegion);
    // Debugging outputs
    //WriteImage<TImage>(this->Image, "InitialImage.mhd");
    //WriteImage<UnsignedCharImageType>(this->Mask, "InitialMask.mhd");
    //WriteImage<FloatImageType>(this->ConfidenceImage, "InitialConfidence.mhd");
    //WriteImage<VectorImageType>(this->IsophoteImage, "InitialIsophotes.mhd");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Initialize()!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::Iterate()
{
  DebugMessage("Iterate()");
  
  FindBoundary();
  Helpers::DebugWriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/BoundaryImage.mha", this->DebugImages);

  DebugMessage("Found boundary.");

  // After inspecting the output of TestBoundaryNormals.cpp, it seems that actually with no blurring the boundary normals are most accurate.
  unsigned int blurVariance = 0;
  ComputeBoundaryNormals(blurVariance);
  Helpers::DebugWriteImageConditional<FloatVector2ImageType>(this->BoundaryNormals, "Debug/BoundaryNormals.mha", this->DebugImages);

  DebugMessage("Computed boundary normals.");

  ComputeAllDataTerms();
  ComputeAllConfidenceTerms();
  ComputeAllPriorities();
  DebugMessage("Computed priorities.");

  PatchPair usedPatchPair;
    
  //FindBestPatchForHighestPriority(sourcePatch, targetPatch);
  
  FindBestPatchLookAhead(usedPatchPair);

  std::cout << "Used target region: " << usedPatchPair.TargetPatch.Region << std::endl;
  
  std::stringstream ssTargetIsophotes;
  ssTargetIsophotes << "Debug/TargetIsophotes_" << this->NumberOfCompletedIterations << ".mha";
  //Helpers::WriteRegion<FloatVector2ImageType>(this->IsophoteImage, usedPatchPair.TargetPatch.Region, ssTargetIsophotes.str());
  Helpers::Write2DVectorRegion(this->IsophoteImage, usedPatchPair.TargetPatch.Region, ssTargetIsophotes.str());
  
  std::stringstream ssSource;
  ssSource << "Debug/source_" << Helpers::ZeroPad(this->NumberOfCompletedIterations, 3) << ".mha";
  Helpers::WritePatch<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.SourcePatch, ssSource.str());

  std::stringstream ssTarget;
  ssTarget << "Debug/target_" << Helpers::ZeroPad(this->NumberOfCompletedIterations, 3) << ".mha";
  //Helpers::WritePatch<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.TargetPatch, ssTarget.str());
  Helpers::WriteRegionUnsignedChar<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.TargetPatch.Region, ssTarget.str());
  
  this->UsedPatchPairs.push_back(usedPatchPair);
  
  // Copy the patch. This is the actual inpainting step.
  Helpers::CopySelfPatchIntoValidRegion<FloatVectorImageType>(this->CurrentOutputImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  
  // We also have to copy patches in the blurred image and CIELab image incase we are using those
  Helpers::CopySelfPatchIntoValidRegion<FloatVectorImageType>(this->BlurredImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  Helpers::CopySelfPatchIntoValidRegion<FloatVectorImageType>(this->CIELabImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  
  float confidence = this->ConfidenceImage->GetPixel(Helpers::GetRegionCenter(usedPatchPair.TargetPatch.Region));
  // Copy the new confidences into the confidence image
  UpdateConfidences(usedPatchPair.TargetPatch.Region, confidence);

  // The isophotes can be copied because they would (should!) only change slightly if recomputed. !!! TODO: Maybe they should actually be recomputed?
  Helpers::CopySelfPatchIntoValidRegion<FloatVector2ImageType>(this->IsophoteImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);

  // Update the mask
  this->UpdateMask(usedPatchPair.TargetPatch.Region);
  DebugMessage("Updated mask.");

  // Sanity check everything
  DebugWriteAllImages();

  // Add new source patches
  // Get the region of pixels which were previous touching the hole which was the target region.
  
  // Shift the top left corner to a position where the same size patch would overlap only the top left pixel.
  itk::Index<2> previouInvalidRegionIndex;
  previouInvalidRegionIndex[0] = usedPatchPair.TargetPatch.Region.GetIndex()[0] - this->PatchRadius[0];
  previouInvalidRegionIndex[1] = usedPatchPair.TargetPatch.Region.GetIndex()[1] - this->PatchRadius[1];
  
  // The region from which patches overlap the used target patch has a radius 2x bigger than the original patch.
  // The computation could be written as (2 * this->PatchRadius[0]) * 2 + 1, or simply this->PatchRadius[0] * 4 + 1
  itk::Size<2> previouInvalidRegionSize;
  previouInvalidRegionSize[0] = this->PatchRadius[0] * 4 + 1;
  previouInvalidRegionSize[1] = this->PatchRadius[1] * 4 + 1;
  
  itk::ImageRegion<2> previousInvalidRegion(previouInvalidRegionIndex, previouInvalidRegionSize);
  
  std::cout << "Used target region: " << usedPatchPair.TargetPatch.Region << std::endl;
  std::cout << "Previously invalid region: " << previousInvalidRegion << std::endl;
  
  std::vector<Patch> newPatches = AddSourcePatches(previousInvalidRegion);
  
  // Recompute for all forward look candidates except the one that was used. Otherwise there would be an exact match!
  
  for(unsigned int candidateId = 0; candidateId < this->PotentialCandidatePairs.size(); ++candidateId)
    {
    // Don't recompute for the target patch that was used.
    if(usedPatchPair.TargetPatch.Region != this->PotentialCandidatePairs[candidateId].TargetPatch.Region)
      {
      this->PotentialCandidatePairs[candidateId].AddPairsFromPatches(newPatches);
      ComputeAllContinuationDifferences(this->PotentialCandidatePairs[candidateId]);
    
      SelfPatchCompare* patchCompare;
      patchCompare = new SelfPatchCompareAll(this->CompareImage->GetNumberOfComponentsPerPixel(), this->PotentialCandidatePairs[candidateId]);
      patchCompare->SetImage(this->CompareImage);
      patchCompare->SetMask(this->CurrentMask);
      patchCompare->ComputeAllDifferences();
      }
    }
  
  // At the end of an iteration, things would be slightly out of sync. The boundary is computed at the beginning of the iteration (before the patch is filled),
  // so at the end of the iteration, the boundary image will not correspond to the boundary of the remaining hole in the image - it will be off by 1 iteration.
  // We fix this by computing the boundary and boundary normals again at the end of the iteration.
  FindBoundary();
  ComputeBoundaryNormals(blurVariance);
  
  this->NumberOfCompletedIterations++;

  DebugMessage<unsigned int>("Completed iteration: ", this->NumberOfCompletedIterations);

}

void CriminisiInpainting::FindBestPatchForHighestPriority(PatchPair& bestPatchPair)
{
  // This function implements Criminisi's idea of "find the highest priority pixel and proceed to fill it".
  // We have replaced this idea with FindBestPatchLookAhead().
  
  // This function returns the best PatchPair by reference.
#if 0
  float highestPriority = 0;
  itk::Index<2> pixelToFill = FindHighestValueOnBoundary(this->PriorityImage, highestPriority);
  DebugMessage<itk::Index<2> >("Highest priority found to be ", pixelToFill);

  itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
  Patch targetPatch;
  targetPatch.Region = targetRegion;
  
  DebugMessage("Finding best patch...");

  CandidatePairs candidatePairs;
  SelfPatchCompare* patchCompare;
  patchCompare = new SelfPatchCompareColor(this->CompareImage->GetNumberOfComponentsPerPixel(), candidatePairs);
  patchCompare->SetImage(this->CompareImage);
  patchCompare->SetMask(this->CurrentMask);
  
  float distance = 0;
  unsigned int bestMatchSourcePatchId = patchCompare->FindBestPatch(distance);
  //DebugMessage<unsigned int>("Found best patch to be ", bestMatchSourcePatchId);
  //std::cout << "Found best patch to be " << bestMatchSourcePatchId << std::endl;

  //this->DebugWritePatch(this->SourcePatches[bestMatchSourcePatchId], "SourcePatch.png");
  //this->DebugWritePatch(targetRegion, "TargetPatch.png");
  Patch sourcePatch;
  sourcePatch = this->SourcePatches[bestMatchSourcePatchId];
  
  bestPatchPair.TargetPatch = targetPatch;
  bestPatchPair.SourcePatch = sourcePatch;
#endif
}


void CriminisiInpainting::FindBestPatchLookAhead(PatchPair& bestPatchPair)
{
  DebugMessage("FindBestPatchLookAhead()");
  // This function returns the best PatchPair by reference
  
  std::cout << "There are " << this->SourcePatches.size() << " source patches at the beginning." << std::endl;
  
  // If this is not the first iteration, get the potential forward look patch candidates from the previous step
  if(this->NumberOfCompletedIterations > 0)
    {
    // Remove the patch candidate that was actually filled
    unsigned int idToRemove = 0;
    for(unsigned int forwardLookId = 0; forwardLookId < this->PotentialCandidatePairs.size(); ++forwardLookId)
      {
      if(this->UsedPatchPairs[this->NumberOfCompletedIterations - 1].TargetPatch.Region == this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region)
	{
	idToRemove = forwardLookId;
	break;
	}
      }
    this->PotentialCandidatePairs.erase(this->PotentialCandidatePairs.begin() + idToRemove);
    DebugMessage<unsigned int>("Removed forward look: ", idToRemove);
    }
  
  // We need to temporarily modify the priority image without affecting the actual priority image, so we copy it.
  FloatScalarImageType::Pointer currentPriorityImage = FloatScalarImageType::New();
  Helpers::DeepCopy<FloatScalarImageType>(this->PriorityImage, currentPriorityImage);

  // Blank all regions that are already look ahead patches.
  for(unsigned int forwardLookId = 0; forwardLookId < this->PotentialCandidatePairs.size(); ++forwardLookId)
    {
    Helpers::SetRegionToConstant<FloatScalarImageType>(currentPriorityImage, this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region, 0.0f);
    }
    
  // Find the remaining number of patch candidates
  unsigned int numberOfNewPatchesToFind = this->MaxForwardLookPatches - this->PotentialCandidatePairs.size();
  for(unsigned int newPatchId = 0; newPatchId < numberOfNewPatchesToFind; ++newPatchId)
    {
    float highestPriority = 0;
    itk::Index<2> pixelToFill = FindHighestValueOnBoundary(currentPriorityImage, highestPriority);

    if(!Helpers::HasHoleNeighbor(pixelToFill, this->CurrentMask))
      {
      std::cerr << "pixelToFill " << pixelToFill << " does not have a hole neighbor - something is wrong!" << std::endl;
      std::cerr << "Mask value " << static_cast<unsigned int>(this->CurrentMask->GetPixel(pixelToFill)) << std::endl;
      std::cerr << "Boundary value " << static_cast<unsigned int>(this->BoundaryImage->GetPixel(pixelToFill)) << std::endl;
      exit(-1);
      }

    // We want to do at least one comparison, but if we are near the end of the completion, there may not be more than 1 priority max
    if(highestPriority < .0001 && newPatchId > 0)
      {
      std::cout << "Highest priority was only " << highestPriority << std::endl;
      break;
      }
    //DebugMessage<itk::Index<2> >("Highest priority found to be ", pixelToFill);

    itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
    Patch targetPatch(targetRegion);

    CandidatePairs candidatePairs(targetPatch);
    candidatePairs.AddPairsFromPatches(this->SourcePatches);
    candidatePairs.Priority = highestPriority;
    
    SelfPatchCompare* patchCompare;
    //patchCompare = new SelfPatchCompareColor(this->CompareImage->GetNumberOfComponentsPerPixel(), candidatePairs);
    patchCompare = new SelfPatchCompareAll(this->CompareImage->GetNumberOfComponentsPerPixel(), candidatePairs);
    patchCompare->SetImage(this->CompareImage);
    patchCompare->SetMask(this->CurrentMask);
    patchCompare->ComputeAllDifferences();
    /*
    float averageSSD = 0;
    unsigned int bestMatchSourcePatchId = patchCompare->FindBestPatch(averageSSD);
    patchPair.AverageSSD = averageSSD;
    
    //DebugMessage<unsigned int>("Found best patch to be ", bestMatchSourcePatchId);
    //std::cout << "Found best patch to be " << bestMatchSourcePatchId << std::endl;

    //this->DebugWritePatch(this->SourcePatches[bestMatchSourcePatchId], "SourcePatch.png");
    //this->DebugWritePatch(targetRegion, "TargetPatch.png");
    Patch sourcePatch = this->SourcePatches[bestMatchSourcePatchId];
    */
    
    ComputeAllContinuationDifferences(candidatePairs);

    // Keep only the number of top patches specified.
    //patchPairsSortedByContinuation.erase(patchPairsSortedByContinuation.begin() + this->NumberOfTopPatchesToSave, patchPairsSortedByContinuation.end());
    
    // Blank a region around the current potential patch to fill. This will ensure the next potential patch to fill is reasonably far away.
    Helpers::SetRegionToConstant<FloatScalarImageType>(currentPriorityImage, targetRegion, 0.0f);
    
    //std::cout << "Potential pair: Source: " << sourcePatch.Region << " Target: " << targetPatch.Region << std::endl;
    
    std::sort(candidatePairs.begin(), candidatePairs.end(), SortByAverageSSD);
    //std::sort(candidatePairs.begin(), candidatePairs.end(), SortByContinuationDifference());
    std::cout << "Sorted " << candidatePairs.size() << " candidatePairs." << std::endl;
    
    this->PotentialCandidatePairs.push_back(candidatePairs);

    } // end forward look loop

  // Sort the forward look patches so that the highest priority sets are first in the vector.
  std::sort(this->PotentialCandidatePairs.rbegin(), this->PotentialCandidatePairs.rend(), SortByPriority);

//   std::cout << "Scores: " << std::endl;
//   for(unsigned int i = 0; i < ssdScores.size(); ++i)
//     {
//     std::cout << i << ": " << ssdScores[i] << std::endl;
//     }
  
  /*
  std::sort(patchPairs.begin(), patchPairs.end(), SortByAverageSSD);
  
  float histogramDifferenceThreshold = .4;
  
  unsigned int bestAttempt = 0;
  
  for(unsigned int i = 0; i < patchPairs.size(); ++i)
    {
    if(patchPairs[i].HistogramDifference < histogramDifferenceThreshold)
      {
      bestAttempt = i;
      break;
      }
    }
    
  std::cout << "Best attempt was " << bestAttempt << std::endl;
  */

  // Choose the look ahead with the lowest score to actually fill rather than simply returning the best source patch of the first look ahead target patch.
  float lowestScore = std::numeric_limits< float >::max();
  unsigned int lowestLookAhead = 0;
  for(unsigned int i = 0; i < this->PotentialCandidatePairs.size(); ++i)
    {
    if(this->PotentialCandidatePairs[i][0].GetAverageSSD() < lowestScore)
      {
      lowestScore = this->PotentialCandidatePairs[i][0].GetAverageSSD();
      lowestLookAhead = i;
      }
    }

  // Return the result by reference.
  bestPatchPair = this->PotentialCandidatePairs[lowestLookAhead][0];
  
  std::cout << "There are " << this->SourcePatches.size() << " source patches at the end." << std::endl;
}

void CriminisiInpainting::Inpaint()
{
  // This function is intended to be used by the command line version. It will do the complete inpainting without updating any UI or the ability to stop before it is complete.
  //std::cout << "CriminisiInpainting::Inpaint()" << std::endl;
  try
  {
    // Start the procedure
    //DebugMessage("Initializing...");
    //Initialize();

    this->NumberOfCompletedIterations = 0;
    while(HasMoreToInpaint())
      {
      Iterate();
      }

  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Inpaint!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::ComputeMaskedIsophotes(FloatScalarImageType::Pointer image, Mask::Pointer mask)
{
  try
  {
    Helpers::DebugWriteImageConditional<FloatScalarImageType>(image, "Debug/ComputeMaskedIsophotes.luminance.mha", this->DebugImages);

    // Compute the gradient
    FloatScalarImageType::Pointer xDerivative = FloatScalarImageType::New();
    Helpers::MaskedDerivative<FloatScalarImageType>(image, mask, 0, xDerivative);

    FloatScalarImageType::Pointer yDerivative = FloatScalarImageType::New();
    Helpers::MaskedDerivative<FloatScalarImageType>(image, mask, 1, yDerivative);

    FloatVector2ImageType::Pointer gradient = FloatVector2ImageType::New();
    Helpers::GradientFromDerivatives<float>(xDerivative, yDerivative, gradient);

    //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(gradient, "Debug/ComputeMaskedIsophotes.gradient.mha", this->DebugImages);
    if(this->DebugImages)
      {
      Helpers::Write2DVectorImage(gradient, "Debug/ComputeMaskedIsophotes.gradient.mha");
      }

    // Rotate the gradient 90 degrees to obtain isophotes from gradient
    typedef itk::UnaryFunctorImageFilter<FloatVector2ImageType, FloatVector2ImageType,
    RotateVectors<FloatVector2ImageType::PixelType,
                  FloatVector2ImageType::PixelType> > FilterType;

    FilterType::Pointer rotateFilter = FilterType::New();
    rotateFilter->SetInput(gradient);
    rotateFilter->Update();

    //Helpers::DebugWriteImageConditional<FloatVector2ImageType>(rotateFilter->GetOutput(), "Debug/ComputeMaskedIsophotes.Isophotes.mha", this->DebugImages);
    if(this->DebugImages)
      {
      Helpers::Write2DVectorImage(rotateFilter->GetOutput(), "Debug/ComputeMaskedIsophotes.Isophotes.mha");
      }

    // Store the result as a member variable.
    Helpers::DeepCopy<FloatVector2ImageType>(rotateFilter->GetOutput(), this->IsophoteImage);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeMaskedIsophotes!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

bool CriminisiInpainting::HasMoreToInpaint()
{
  try
  {
    if(this->DebugImages)
      {
      Helpers::WriteImage<Mask>(this->CurrentMask, "Debug/HasMoreToInpaint.input.png");
      }
      
    itk::ImageRegionIterator<Mask> maskIterator(this->CurrentMask, this->CurrentMask->GetLargestPossibleRegion());

    while(!maskIterator.IsAtEnd())
      {
      if(this->CurrentMask->IsHole(maskIterator.GetIndex()))
	{
	return true;
	}

      ++maskIterator;
      }

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

void CriminisiInpainting::FindBoundary()
{
  try
  {
    // Compute the "outer" boundary of the region to fill. That is, we want the boundary pixels to be in the source region.

    Helpers::DebugWriteImageConditional<Mask>(this->CurrentMask, "Debug/FindBoundary.CurrentMask.mha", this->DebugImages);
    Helpers::DebugWriteImageConditional<Mask>(this->CurrentMask, "Debug/FindBoundary.CurrentMask.png", this->DebugImages);

    // Create a binary image (throw away the "dont use" pixels)
    Mask::Pointer holeOnly = Mask::New();
    holeOnly->DeepCopyFrom(this->CurrentMask);
    
    itk::ImageRegionIterator<Mask> maskIterator(holeOnly, holeOnly->GetLargestPossibleRegion());
    // This should result in a white hole on a black background
    while(!maskIterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = maskIterator.GetIndex();
      if(!holeOnly->IsHole(currentPixel))
	{
	holeOnly->SetPixel(currentPixel, holeOnly->GetValidValue());
	}
      ++maskIterator;
      }

    Helpers::DebugWriteImageConditional<Mask>(holeOnly, "Debug/FindBoundary.HoleOnly.mha", this->DebugImages);
    Helpers::DebugWriteImageConditional<Mask>(holeOnly, "Debug/FindBoundary.HoleOnly.png", this->DebugImages);
      
    // Since the hole is white, we want the foreground value of the contour filter to be black. This means that the boundary will
    // be detected in the black pixel region, which is on the outside edge of the hole like we want. However,
    // The BinaryContourImageFilter will change all non-boundary pixels to the background color, so the resulting output will be inverted -
    // the boundary pixels will be black and the non-boundary pixels will be white.
    
    // Find the boundary
    typedef itk::BinaryContourImageFilter <Mask, Mask> binaryContourImageFilterType;
    binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New();
    binaryContourFilter->SetInput(holeOnly);
    binaryContourFilter->SetFullyConnected(true);
    binaryContourFilter->SetForegroundValue(holeOnly->GetValidValue());
    binaryContourFilter->SetBackgroundValue(holeOnly->GetHoleValue());
    binaryContourFilter->Update();

    Helpers::DebugWriteImageConditional<Mask>(binaryContourFilter->GetOutput(), "Debug/FindBoundary.Boundary.mha", this->DebugImages);
    Helpers::DebugWriteImageConditional<Mask>(binaryContourFilter->GetOutput(), "Debug/FindBoundary.Boundary.png", this->DebugImages);

    // Since we want to interpret non-zero pixels as boundary pixels, we must invert the image.
    typedef itk::InvertIntensityImageFilter <Mask> InvertIntensityImageFilterType;
    InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
    invertIntensityFilter->SetInput(binaryContourFilter->GetOutput());
    invertIntensityFilter->SetMaximum(255);
    invertIntensityFilter->Update();
    
    //this->BoundaryImage = binaryContourFilter->GetOutput();
    //this->BoundaryImage->Graft(binaryContourFilter->GetOutput());
    Helpers::DeepCopy<UnsignedCharScalarImageType>(invertIntensityFilter->GetOutput(), this->BoundaryImage);

    Helpers::DebugWriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/FindBoundary.BoundaryImage.mha", this->DebugImages);
    
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindBoundary!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::UpdateMask(const itk::ImageRegion<2>& region)
{
  DebugMessage("UpdateMask()");
  try
  {
    itk::ImageRegionIterator<Mask> maskIterator(this->CurrentMask, region);
  
    while(!maskIterator.IsAtEnd())
      {
      if(this->CurrentMask->IsHole(maskIterator.GetIndex()))
	{
	maskIterator.Set(this->CurrentMask->GetValidValue());
	}
  
      ++maskIterator;
      }
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateMask!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::ComputeBoundaryNormals(const unsigned int blurVariance)
{
  DebugMessage("ComputeBoundaryNormals()");
  try
  {
    // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

    Helpers::DebugWriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/ComputeBoundaryNormals.BoundaryImage.mha", this->DebugImages);
    Helpers::DebugWriteImageConditional<Mask>(this->CurrentMask, "Debug/ComputeBoundaryNormals.CurrentMask.mha", this->DebugImages);
      
    // Blur the mask
    typedef itk::DiscreteGaussianImageFilter< Mask, FloatScalarImageType >  BlurFilterType;
    BlurFilterType::Pointer gaussianFilter = BlurFilterType::New();
    gaussianFilter->SetInput(this->CurrentMask);
    gaussianFilter->SetVariance(blurVariance);
    gaussianFilter->Update();

    Helpers::DebugWriteImageConditional<FloatScalarImageType>(gaussianFilter->GetOutput(), "Debug/ComputeBoundaryNormals.BlurredMask.mha", this->DebugImages);

    // Compute the gradient of the blurred mask
    typedef itk::GradientImageFilter< FloatScalarImageType, float, float>  GradientFilterType;
    GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
    gradientFilter->SetInput(gaussianFilter->GetOutput());
    gradientFilter->Update();

    Helpers::DebugWriteImageConditional<FloatVector2ImageType>(gradientFilter->GetOutput(), "Debug/ComputeBoundaryNormals.BlurredMaskGradient.mha", this->DebugImages);

    // Only keep the normals at the boundary
    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput(gradientFilter->GetOutput());
    maskFilter->SetMaskImage(this->BoundaryImage);
    maskFilter->Update();

    Helpers::DebugWriteImageConditional<FloatVector2ImageType>(maskFilter->GetOutput(), "Debug/ComputeBoundaryNormals.BoundaryNormalsUnnormalized.mha", this->DebugImages);

    Helpers::DeepCopy<FloatVector2ImageType>(maskFilter->GetOutput(), this->BoundaryNormals);

    // Normalize the vectors because we just care about their direction (the Data term computation calls for the normalized boundary normal)
    itk::ImageRegionIterator<FloatVector2ImageType> boundaryNormalsIterator(this->BoundaryNormals, this->BoundaryNormals->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    while(!boundaryNormalsIterator.IsAtEnd())
      {
      if(boundaryIterator.Get()) // The pixel is on the boundary
        {
        FloatVector2ImageType::PixelType p = boundaryNormalsIterator.Get();
        p.Normalize();
        boundaryNormalsIterator.Set(p);
        }
      ++boundaryNormalsIterator;
      ++boundaryIterator;
      }

    Helpers::DebugWriteImageConditional<FloatVector2ImageType>(this->BoundaryNormals, "Debug/ComputeBoundaryNormals.BoundaryNormals.mha", this->DebugImages);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeBoundaryNormals!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

itk::Index<2> CriminisiInpainting::FindHighestValueOnBoundary(const FloatScalarImageType::Pointer image, float& maxValue)
{
  // Return the location of the highest pixel on the current boundary. Return the value of that pixel by reference.
  try
  {
    // Explicity find the maximum on the boundary
    maxValue = 0; // priorities are non-negative, so anything better than 0 will win
    itk::Index<2> locationOfMaxValue;
    itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    while(!boundaryIterator.IsAtEnd())
      {
      if(boundaryIterator.Get())
	{
	if(image->GetPixel(boundaryIterator.GetIndex()) > maxValue)
	  {
	  maxValue = image->GetPixel(boundaryIterator.GetIndex());
	  locationOfMaxValue = boundaryIterator.GetIndex();
	  }
	}
      ++boundaryIterator;
      }
    DebugMessage<float>("Highest value: ", maxValue);
    return locationOfMaxValue;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindHighestValueOnBoundary!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::ComputeAllPriorities()
{
  try
  {
    // Only compute priorities for pixels on the boundary
    itk::ImageRegionConstIterator<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    // Blank the priority image.
    this->PriorityImage->FillBuffer(0);

    // The main loop is over the boundary image. We only want to compute priorities at boundary pixels.
    unsigned int boundaryPixelCounter = 0;
    while(!boundaryIterator.IsAtEnd())
      {
      if(boundaryIterator.Get() != 0) // Pixel is on the boundary
	{
	float priority = ComputePriority(boundaryIterator.GetIndex());
	//DebugMessage<float>("Priority: ", priority);
	this->PriorityImage->SetPixel(boundaryIterator.GetIndex(), priority);
	boundaryPixelCounter++;
	}    
      ++boundaryIterator;
      }
    DebugMessage<unsigned int>("Number of boundary pixels: ", boundaryPixelCounter);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllPriorities!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

float CriminisiInpainting::ComputePriority(const itk::Index<2>& queryPixel)
{
  //double confidence = ComputeConfidenceTerm(queryPixel);
  //double data = ComputeDataTerm(queryPixel);

  float confidence = this->ConfidenceImage->GetPixel(queryPixel);
  float data = this->DataImage->GetPixel(queryPixel);

  float priority = confidence * data;

  return priority;
}

float CriminisiInpainting::ComputeConfidenceTerm(const itk::Index<2>& queryPixel)
{
  //DebugMessage<itk::Index<2>>("Computing confidence for ", queryPixel);
  try
  {
    // Allow for regions on/near the image border

    //itk::ImageRegion<2> region = this->CurrentMask->GetLargestPossibleRegion();
    //region.Crop(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
    itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]);
    itk::ImageRegion<2> region = CropToValidRegion(targetRegion);
    
    itk::ImageRegionConstIterator<Mask> maskIterator(this->CurrentMask, region);

    // The confidence is computed as the sum of the confidences of patch pixels in the source region / area of the patch

    float sum = 0;

    while(!maskIterator.IsAtEnd())
      {
      if(this->CurrentMask->IsValid(maskIterator.GetIndex()))
        {
        sum += this->ConfidenceMapImage->GetPixel(maskIterator.GetIndex());
        }
      ++maskIterator;
      }

    unsigned int numberOfPixels = GetNumberOfPixelsInPatch();
    float areaOfPatch = static_cast<float>(numberOfPixels);
    
    float confidence = sum/areaOfPatch;
    DebugMessage<float>("Confidence: ", confidence);

    return confidence;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeConfidenceTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

float CriminisiInpainting::ComputeDataTerm(const itk::Index<2>& queryPixel)
{
  try
  {
    FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
    FloatVector2Type boundaryNormal = this->BoundaryNormals->GetPixel(queryPixel);

    DebugMessage<FloatVector2Type>("Isophote: ", isophote);
    DebugMessage<FloatVector2Type>("Boundary normal: ", boundaryNormal);
    // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

    vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

    vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

    float dot = std::abs(dot_product(vnlIsophote,vnlNormal));

    float alpha = 255; // This doesn't actually contribue anything, since the argmax of the priority is all that is used, and alpha ends up just being a scaling factor since the proiority is purely multiplicative.
    float dataTerm = dot/alpha;

    return dataTerm;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::UpdateConfidences(const itk::ImageRegion<2>& targetRegion, const float value)
{
  DebugMessage("UpdateConfidences()");
  try
  {
    // Force the region to update to be entirely inside the image
    itk::ImageRegion<2> region = CropToValidRegion(targetRegion);
    
    // Use an iterator to find masked pixels. Compute their new value, and save it in a vector of pixels and their new values.
    // Do not update the pixels until after all new values have been computed, because we want to use the old values in all of
    // the computations.
    itk::ImageRegionConstIterator<Mask> maskIterator(this->CurrentMask, region);

    while(!maskIterator.IsAtEnd())
      {
      if(this->CurrentMask->IsHole(maskIterator.GetIndex()))
	{
	itk::Index<2> currentPixel = maskIterator.GetIndex();
	this->ConfidenceMapImage->SetPixel(currentPixel, value);
	}

      ++maskIterator;
      } // end while looop with iterator

  } // end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateConfidences!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void CriminisiInpainting::ComputeAllDataTerms()
{
  try
  {
    itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    // Blank the data term image
    this->DataImage->FillBuffer(0);

    while(!boundaryIterator.IsAtEnd())
      {
      if(boundaryIterator.Get() != 0) // This is a pixel on the current boundary
	{
	itk::Index<2> currentPixel = boundaryIterator.GetIndex();
	float dataTerm = ComputeDataTerm(currentPixel);
	this->DataImage->SetPixel(currentPixel, dataTerm);
	//DebugMessage<float>("Set DataTerm to ", dataTerm);
	}

      ++boundaryIterator;
      }
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllDataTerms!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void CriminisiInpainting::ComputeAllConfidenceTerms()
{
  try
  {
    itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    // Blank the data term image
    this->ConfidenceImage->FillBuffer(0);

    while(!boundaryIterator.IsAtEnd())
      {
      if(boundaryIterator.Get() != 0) // This is a pixel on the current boundary
	{
	itk::Index<2> currentPixel = boundaryIterator.GetIndex();
	float confidenceTerm = ComputeConfidenceTerm(currentPixel);
	this->ConfidenceImage->SetPixel(currentPixel, confidenceTerm);
	}

      ++boundaryIterator;
      }
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllConfidenceTerms!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

bool CriminisiInpainting::IsValidPatch(const itk::Index<2>& queryPixel, const unsigned int radius)
{
  // This function checks if a patch is completely inside the image and not intersecting the mask

  itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(queryPixel, radius);
  return IsValidRegion(region);
}

bool CriminisiInpainting::IsValidRegion(const itk::ImageRegion<2>& region)
{
  return this->CurrentMask->IsValid(region);
}

unsigned int CriminisiInpainting::GetNumberOfPixelsInPatch()
{
  return this->GetPatchSize()[0]*this->GetPatchSize()[1];
}

itk::Size<2> CriminisiInpainting::GetPatchSize()
{
  itk::Size<2> patchSize;

  patchSize[0] = Helpers::SideLengthFromRadius(this->PatchRadius[0]);
  patchSize[1] = Helpers::SideLengthFromRadius(this->PatchRadius[1]);

  return patchSize;
}

itk::ImageRegion<2> CriminisiInpainting::CropToValidRegion(const itk::ImageRegion<2>& inputRegion)
{
  itk::ImageRegion<2> region = this->FullImageRegion;
  region.Crop(inputRegion);
  
  return region;
}

CandidatePairs& CriminisiInpainting::GetPotentialCandidatePairReference(const unsigned int forwardLookId)
{
  return this->PotentialCandidatePairs[forwardLookId];
}

std::vector<CandidatePairs> CriminisiInpainting::GetPotentialCandidatePairs()
{
  return this->PotentialCandidatePairs;
}

bool CriminisiInpainting::GetUsedPatchPair(const unsigned int id, PatchPair& patchPair)
{
  if(id < this->UsedPatchPairs.size())
    {
    patchPair = this->UsedPatchPairs[id]; 
    return true;
    }
  return false;
}
  
unsigned int CriminisiInpainting::GetNumberOfCompletedIterations()
{
  return this->NumberOfCompletedIterations;
}

bool CriminisiInpainting::GetAdjacentBoundaryPixel(const itk::Index<2>& boundaryPixel, const PatchPair& patchPair, itk::Index<2>& adjacentBoundaryPixel)
{
  if(this->CurrentMask->IsHole(boundaryPixel) || !patchPair.TargetPatch.Region.IsInside(boundaryPixel))
    {
    std::cerr << "Error: The input boundary pixel must be on the valid side of the boundary (not in the hole)!" << std::endl;
    exit(-1);
    }
  
  FloatVector2Type sourceSideIsophote = this->IsophoteImage->GetPixel(boundaryPixel);
    
  itk::Index<2> pixelAcrossBoundary = Helpers::GetNextPixelAlongVector(boundaryPixel, sourceSideIsophote);
  
  // Some pixels might not have a valid pixel on the other side of the boundary.
  bool valid = false;
  
  // If the next pixel along the isophote is in bounds and in the hole region of the patch, procede.
  if(patchPair.TargetPatch.Region.IsInside(pixelAcrossBoundary) && this->CurrentMask->IsHole(pixelAcrossBoundary))
    {
    valid = true;
    }
  else
    {
    // There is no requirement for the isophote to be pointing a particular orientation, so try to step along the negative isophote.
    sourceSideIsophote *= -1.0;
    pixelAcrossBoundary = Helpers::GetNextPixelAlongVector(boundaryPixel, sourceSideIsophote);
    if(patchPair.TargetPatch.Region.IsInside(pixelAcrossBoundary) && this->CurrentMask->IsHole(pixelAcrossBoundary))
      {
      valid = true;
      }
    }
    
  if(!valid)
    {
    return valid;
    }

  // Determine the position of the pixel relative to the patch corner.
  itk::Offset<2> intraPatchOffset = pixelAcrossBoundary - patchPair.TargetPatch.Region.GetIndex();

  // Determine the position of the corresponding pixel in the source patch.
  itk::Index<2> sourcePatchTargetPixel = patchPair.SourcePatch.Region.GetIndex() + intraPatchOffset;
  
  // Return by reference
  adjacentBoundaryPixel = sourcePatchTargetPixel;
  
  return valid;
}

float CriminisiInpainting::ComputeIsophoteDifference(const itk::Index<2>& pixel1, const itk::Index<2>& pixel2)
{
  // Get the isophote on the hole side of the boundary
  FloatVector2Type isophote1 = this->IsophoteImage->GetPixel(pixel1);
  FloatVector2Type isophote2 = this->IsophoteImage->GetPixel(pixel2);
  
  return ComputeIsophoteDifference(isophote1, isophote2);
}

float CriminisiInpainting::ComputeIsophoteDifference(const FloatVector2Type v1, const FloatVector2Type v2)
{
  // Compute the isophote difference.
  float isophoteDifference = Helpers::AngleBetween(v1, v2);
  
  float isophoteDifferenceNormalized = isophoteDifference/3.14159; // The maximum angle between vectors is pi, so this produces a score between 0 and 1.
  DebugMessage<float>("isophoteDifferenceNormalized: ", isophoteDifferenceNormalized);
  
  return isophoteDifferenceNormalized;
}

float CriminisiInpainting::ComputeNormalizedSquaredPixelDifference(const itk::Index<2>& pixel1, const itk::Index<2>& pixel2)
{
  // Compute the pixel difference.
      
  FloatVectorImageType::PixelType value1 = this->CompareImage->GetPixel(pixel1);
  FloatVectorImageType::PixelType value2 = this->CompareImage->GetPixel(pixel2);
  DebugMessage<FloatVectorImageType::PixelType>("value1 ", value1);
  DebugMessage<FloatVectorImageType::PixelType>("value2 ", value2);
  
  float pixelSquaredDifference = SelfPatchCompareAll::StaticPixelDifferenceSquared(this->CompareImage->GetPixel(pixel1), this->CompareImage->GetPixel(pixel2));
  DebugMessage<float>("ComputePixelDifference::pixelSquaredDifference", pixelSquaredDifference);
  
  //std::cout << "pixelDifference: " << pixelDifference << std::endl;
  //float pixelDifferenceNormalized = pixelDifference / MaxPixelDifference; // This produces a score between 0 and 1.
  DebugMessage<float>("ComputePixelDifference::normFactor ", MaxPixelDifferenceSquared);
  float pixelSquaredDifferenceNormalized = pixelSquaredDifference / MaxPixelDifferenceSquared; // This produces a score between 0 and 1.
  //DebugMessage<float>("pixelDifferenceNormalized: ", pixelDifferenceNormalized);
  
  return pixelSquaredDifferenceNormalized;
}


void CriminisiInpainting::ComputeAllContinuationDifferences(CandidatePairs& candidatePairs)
{
  // Naively, we could just call ComputeTotalContinuationDifference on each patch pair for a forward looking set. However, this
  // would recalculate the boundary for every pair. This is a lot of extra work because the boundary does not change from source patch to source patch.
  
  // Identify border pixels on the source side of the boundary.
  std::vector<itk::Index<2> > borderPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage, candidatePairs.TargetPatch.Region);
  
  for(unsigned int sourcePatchId = 0; sourcePatchId < this->SourcePatches.size(); ++sourcePatchId)
    {
    // Only compute if the values are not already computed.
    if(candidatePairs[sourcePatchId].IsValidBoundaryIsophoteDifference() && candidatePairs[sourcePatchId].IsValidBoundaryPixelDifference())
      {
      continue;
      }
    float totalPixelDifference = 0.0f;
    float totalIsophoteDifference = 0.0f;
    unsigned int numberUsed = 0;
    for(unsigned int pixelId = 0; pixelId < borderPixels.size(); ++pixelId)
      {
      itk::Index<2> currentPixel = borderPixels[pixelId];
      itk::Index<2> adjacentBoundaryPixel;
      bool valid = GetAdjacentBoundaryPixel(currentPixel, candidatePairs[sourcePatchId], adjacentBoundaryPixel);
      if(!valid)
	{
	continue;
	}
      numberUsed++;
      float normalizedSquaredPixelDifference = ComputeNormalizedSquaredPixelDifference(currentPixel, adjacentBoundaryPixel);
      DebugMessage<float>("ComputeAllContinuationDifferences::normalizedSquaredPixelDifference ", normalizedSquaredPixelDifference);
      float isophoteDifference = ComputeIsophoteDifference(currentPixel, adjacentBoundaryPixel);
      totalPixelDifference += normalizedSquaredPixelDifference;
      totalIsophoteDifference += isophoteDifference;
      } // end loop over pixels
      
    DebugMessage<unsigned int>("numberUsed ", numberUsed);
    DebugMessage<unsigned int>("Out of ", borderPixels.size());
    
    float averagePixelDifference = totalPixelDifference / static_cast<float>(numberUsed);
    DebugMessage<float>("averagePixelDifference ", averagePixelDifference);
    
    float averageIsophoteDifference = totalIsophoteDifference / static_cast<float>(numberUsed);
    candidatePairs[sourcePatchId].SetBoundaryPixelDifference(averagePixelDifference);
    candidatePairs[sourcePatchId].SetBoundaryIsophoteDifference(averageIsophoteDifference);
    
    } // end loop over pairs

}

std::vector<CandidatePairs>& CriminisiInpainting::GetPotentialCandidatePairsReference()
{
  // Return a reference to the whole set of forward look pairs.
  return PotentialCandidatePairs;
}
