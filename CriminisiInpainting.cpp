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
#include "Derivatives.h"
#include "Helpers.h"
#include "PixelDifference.h"
#include "SelfPatchCompare.h"

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
#include "itkVectorImageToImageAdaptor.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

CriminisiInpainting::CriminisiInpainting()
{
  //std::cout << "CriminisiInpainting()" << std::endl;
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
  this->LuminanceImage = FloatScalarImageType::New();
  
  // Set the image to use for pixel to pixel comparisons.
  //this->CompareImage = this->CIELabImage;
  //this->CompareImage = this->BlurredImage;
  this->CompareImage = this->CurrentOutputImage;
  
  this->ConfidenceImage = FloatScalarImageType::New();
  this->ConfidenceMapImage = FloatScalarImageType::New();
  this->DataImage = FloatScalarImageType::New();
  
  this->DebugImages = false;
  this->DebugMessages = false;
  this->NumberOfCompletedIterations = 0;
  
  this->HistogramBinsPerDimension = 10;
  this->MaxForwardLookPatches = 10;
  
  this->MaxPixelDifferenceSquared = 0.0f;
  
  this->DebugFunctionEnterLeave = false;
  
  this->PatchSortFunction = &SortByAverageAbsoluteDifference;

  this->PatchCompare = new SelfPatchCompare;
}

void CriminisiInpainting::ComputeMaxPixelDifference()
{
  EnterFunction("ComputeMaxPixelDifference()");
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
  this->MaxPixelDifferenceSquared = FullPixelDifference::Difference(maxPixel, zeroPixel, zeroPixel.GetNumberOfElements());
  
  std::cout << "MaxPixelDifference computed to be: " << this->MaxPixelDifferenceSquared << std::endl;
  LeaveFunction("ComputeMaxPixelDifference()");
}

bool CriminisiInpainting::PatchExists(const itk::ImageRegion<2>& region)
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

std::vector<Patch> CriminisiInpainting::AddNewSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid and are not already in the source patch list to the list of source patches.
  // Additionally, return the patches that were added.
  EnterFunction("AddNewSourcePatchesInRegion()");
  
  std::vector<Patch> newPatches;
  try
  {
    // Clearly we cannot add source patches from regions that are outside the image.
    itk::ImageRegion<2> newRegion = CropToValidRegion(region);

    itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->CurrentOutputImage, newRegion);

    while(!imageIterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = imageIterator.GetIndex();
      itk::ImageRegion<2> currentPatchRegion = Helpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius[0]);
    
      if(this->CurrentMask->GetLargestPossibleRegion().IsInside(currentPatchRegion))
	{
	if(this->CurrentMask->IsValid(currentPatchRegion))
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

void CriminisiInpainting::AddAllSourcePatchesInRegion(const itk::ImageRegion<2>& region)
{
  // Add all patches in 'region' that are entirely valid to the list of source patches.
    
  EnterFunction("AddAllSourcePatchesInRegion()");
  
  try
  {
    // Clearly we cannot add source patches from regions that are outside the image, so crop the desired region to be inside the image.
    itk::ImageRegion<2> newRegion = CropToValidRegion(region);

    itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(this->CurrentOutputImage, newRegion);

    while(!imageIterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = imageIterator.GetIndex();
      itk::ImageRegion<2> currentPatchRegion = Helpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius[0]);
    
      if(this->CurrentMask->GetLargestPossibleRegion().IsInside(currentPatchRegion))
	{
	if(this->CurrentMask->IsValid(currentPatchRegion))
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

void CriminisiInpainting::InitializeConfidenceMap()
{
  EnterFunction("InitializeConfidenceMap()");
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
  LeaveFunction("InitializeConfidenceMap()");
}

void CriminisiInpainting::InitializeTargetImage()
{
  EnterFunction("InitializeTargetImage()");
  // Initialize to the input
  Helpers::DeepCopy<FloatVectorImageType>(this->OriginalImage, this->CurrentOutputImage);
  LeaveFunction("InitializeTargetImage()");
}

void CriminisiInpainting::ComputeIsophotes()
{
  EnterFunction("ComputeIsophotes()");
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  Helpers::VectorImageToRGBImage(this->OriginalImage, rgbImage);
  
  Helpers::WriteImageConditional<RGBImageType>(rgbImage, "Debug/Initialize.rgb.mha", this->DebugImages);

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();
  
  Helpers::DeepCopy<FloatScalarImageType>(luminanceFilter->GetOutput(), this->LuminanceImage);
  
  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel. From TestIsophotes.cpp, it actually seems like not blurring, but using a masked sobel operator produces the most reliable isophotes.
  unsigned int kernelRadius = 0;
  Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), this->CurrentMask, kernelRadius, blurredLuminance);
  
  Helpers::WriteImageConditional<FloatScalarImageType>(blurredLuminance, "Debug/Initialize.blurredLuminance.mha", true);
  
  Helpers::InitializeImage<FloatVector2ImageType>(this->IsophoteImage, this->FullImageRegion);
  ComputeMaskedIsophotesInRegion(blurredLuminance, this->CurrentMask, this->FullImageRegion, this->IsophoteImage);
  if(this->DebugImages)
    {
    Helpers::Write2DVectorImage(this->IsophoteImage, "Debug/Initialize.IsophoteImage.mha");
    }
  LeaveFunction("ComputeIsophotes()");
}

void CriminisiInpainting::Initialize()
{
  EnterFunction("Initialize()");
  try
  {
    this->NumberOfCompletedIterations = 0;
  
    // We find the boundary of the mask and its normals at every iteration (in Iterate()), but we do this here so that everything is initialized and valid if we are observing this class for display.
    FindBoundary();
    
    unsigned int blurVariance = 2;
    ComputeBoundaryNormals(blurVariance);
    
    InitializeTargetImage();
    Helpers::WriteImageConditional<FloatVectorImageType>(this->CurrentOutputImage, "Debug/Initialize.CurrentOutputImage.mha", this->DebugImages);
  
    ComputeIsophotes();
    
    // Blur the image incase we want to use a blurred image for pixel to pixel comparisons.
    //unsigned int kernelRadius = 5;
    //Helpers::VectorMaskedBlur(this->OriginalImage, this->CurrentMask, kernelRadius, this->BlurredImage);
    
    // BlurImage();

    // Initialize internal images
    Helpers::InitializeImage<FloatScalarImageType>(this->DataImage, this->FullImageRegion);
    Helpers::InitializeImage<FloatScalarImageType>(this->PriorityImage, this->FullImageRegion);
    Helpers::InitializeImage<FloatScalarImageType>(this->ConfidenceImage, this->FullImageRegion);
        
    InitializeConfidenceMap();
    Helpers::WriteImageConditional<FloatScalarImageType>(this->ConfidenceMapImage, "Debug/Initialize.ConfidenceMapImage.mha", this->DebugImages);

    DebugMessage("Computing source patches...");
    AddAllSourcePatchesInRegion(this->FullImageRegion);
    // Debugging outputs
    //WriteImage<TImage>(this->Image, "InitialImage.mhd");
    //WriteImage<UnsignedCharImageType>(this->Mask, "InitialMask.mhd");
    //WriteImage<FloatImageType>(this->ConfidenceImage, "InitialConfidence.mhd");
    //WriteImage<VectorImageType>(this->IsophoteImage, "InitialIsophotes.mhd");
    
    if(this->OriginalMask->GetLargestPossibleRegion() != this->OriginalImage->GetLargestPossibleRegion())
      {
      std::cerr << "Original mask size does not match original image size!" << std::endl;
      std::cerr << "Original mask size: " << this->OriginalMask->GetLargestPossibleRegion() << std::endl;
      std::cerr << "Original image size: " << this->OriginalImage->GetLargestPossibleRegion() << std::endl;
      exit(-1);
      }
      
    this->PatchCompare->SetNumberOfComponentsPerPixel(this->CompareImage->GetNumberOfComponentsPerPixel());
    
    LeaveFunction("Initialize()");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Initialize()!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

PatchPair CriminisiInpainting::Iterate()
{
  EnterFunction("Iterate()");
  
  FindBoundary();
  Helpers::WriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/BoundaryImage.mha", this->DebugImages);

  DebugMessage("Found boundary.");

  // The affect of this parameter can be inspected using the output of TestBoundaryNormals.
  unsigned int blurVariance = 2;
  ComputeBoundaryNormals(blurVariance);
  Helpers::WriteImageConditional<FloatVector2ImageType>(this->BoundaryNormals, "Debug/BoundaryNormals.mha", this->DebugImages);

  DebugMessage("Computed boundary normals.");

  ComputeAllDataTerms();
  ComputeAllConfidenceTerms();
  ComputeAllPriorities();
  DebugMessage("Computed priorities.");

  PatchPair usedPatchPair;
    
  FindBestPatchLookAhead(usedPatchPair);
  
  //std::cout << "Used target region: " << usedPatchPair.TargetPatch.Region << std::endl;
  
  if(this->DebugImages)
    {
    std::stringstream ssTargetIsophotes;
    ssTargetIsophotes << "Debug/TargetIsophotes_" << this->NumberOfCompletedIterations << ".mha";
    Helpers::Write2DVectorRegion(this->IsophoteImage, usedPatchPair.TargetPatch.Region, ssTargetIsophotes.str());
      
    
    std::stringstream ssSource;
    ssSource << "Debug/source_" << Helpers::ZeroPad(this->NumberOfCompletedIterations, 3) << ".mha";
    Helpers::WritePatch<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.SourcePatch, ssSource.str());

    std::stringstream ssTargetMHA;
    ssTargetMHA << "Debug/target_" << Helpers::ZeroPad(this->NumberOfCompletedIterations, 3) << ".mha";
    Helpers::WritePatch<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.TargetPatch, ssTargetMHA.str());
    
    std::stringstream ssTargetPNG;
    ssTargetPNG << "Debug/target_" << Helpers::ZeroPad(this->NumberOfCompletedIterations, 3) << ".png";
    Helpers::WriteRegionUnsignedChar<FloatVectorImageType>(this->CurrentOutputImage, usedPatchPair.TargetPatch.Region, ssTargetPNG.str());
    }
  
  // Copy the patch. This is the actual inpainting step.
  Helpers::CopySelfPatchIntoHoleOfTargetRegion<FloatVectorImageType>(this->CurrentOutputImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  
  // We also have to copy patches in the blurred image and CIELab image incase we are using those
  //Helpers::CopySelfPatchIntoHoleOfTargetRegion<FloatVectorImageType>(this->BlurredImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  Helpers::CopySelfPatchIntoHoleOfTargetRegion<FloatVectorImageType>(this->CIELabImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  Helpers::CopySelfPatchIntoHoleOfTargetRegion<FloatScalarImageType>(this->LuminanceImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  
  float confidence = this->ConfidenceImage->GetPixel(Helpers::GetRegionCenter(usedPatchPair.TargetPatch.Region));
  // Copy the new confidences into the confidence image
  UpdateConfidences(usedPatchPair.TargetPatch.Region, confidence);

  // Copy the isophotes under the assumption that they would only change slightly if recomputed
  //Helpers::CopySelfPatchIntoHoleOfTargetRegion<FloatVector2ImageType>(this->IsophoteImage, this->CurrentMask, usedPatchPair.SourcePatch.Region, usedPatchPair.TargetPatch.Region);
  
  //FloatScalarImageType::Pointer newIsophotes = FloatScalarImageType::New();
  //Helpers::InitializeImage<FloatScalarImageType>(newIsophotes, this->FullImageRegion);
  //ComputeMaskedIsophotesInRegion(this->LuminanceImage, this->CurrentMask, newIsophotes);
  //Helpers::CopyPatch(newIsophotes, this->IsophoteImage, usedPatchPair.TargetPatch.Region, usedPatchPair.TargetPatch.Region);

  ComputeMaskedIsophotesInRegion(this->LuminanceImage, this->CurrentMask, usedPatchPair.TargetPatch.Region, this->IsophoteImage);

  // Update the mask
  this->UpdateMask(usedPatchPair.TargetPatch.Region);
  DebugMessage("Updated mask.");

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
  itk::Size<2> previouInvalidRegionSize;
  previouInvalidRegionSize[0] = this->PatchRadius[0] * 4 + 1;
  previouInvalidRegionSize[1] = this->PatchRadius[1] * 4 + 1;
  
  itk::ImageRegion<2> previousInvalidRegion(previousInvalidRegionIndex, previouInvalidRegionSize);
  
  //std::cout << "Used target region: " << usedPatchPair.TargetPatch.Region << std::endl;
  //std::cout << "Previously invalid region: " << previousInvalidRegion << std::endl;
  
  std::vector<Patch> newPatches = AddNewSourcePatchesInRegion(previousInvalidRegion);
  
  
  RecomputeScoresWithNewPatches(newPatches, usedPatchPair);


  // At the end of an iteration, things would be slightly out of sync. The boundary is computed at the beginning of the iteration (before the patch is filled),
  // so at the end of the iteration, the boundary image will not correspond to the boundary of the remaining hole in the image - it will be off by 1 iteration.
  // We fix this by computing the boundary and boundary normals again at the end of the iteration.
  FindBoundary();
  ComputeBoundaryNormals(blurVariance);
  
  this->NumberOfCompletedIterations++;

  DebugMessage<unsigned int>("Completed iteration: ", this->NumberOfCompletedIterations);

  PreviousIterationUsedPatchPair = usedPatchPair;
  
  LeaveFunction("Iterate()");
  return usedPatchPair;
}

void CriminisiInpainting::RecomputeScoresWithNewPatches(std::vector<Patch>& newPatches, PatchPair& usedPatchPair)
{
  EnterFunction("RecomputeScoresWithNewPatches()");
  
  if(newPatches.size() <= 0)
    {
    std::cout << "There were 0 new patches to recompute!" << std::endl;
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
    this->PatchCompare->SetMask(this->CurrentMask);
    this->PatchCompare->ComputeAllSourceDifferences();
    
    this->PotentialCandidatePairs[candidateId].Combine(newPairs);
    }
    
  LeaveFunction("RecomputeScoresWithNewPatches()");
}

void CriminisiInpainting::FindBestPatchScaleConsistent(CandidatePairs& candidatePairs, PatchPair& bestPatchPair)
{
  //std::cout << "FindBestPatchScaleConsistent: There are " << this->SourcePatches.size() << " source patches at the beginning." << std::endl;
  EnterFunction("FindBestPatchScaleConsistent()");
  
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(this->BlurredImage);
  this->PatchCompare->SetMask(this->CurrentMask);
  this->PatchCompare->ComputeAllSourceDifferences();
  
  std::sort(candidatePairs.begin(), candidatePairs.end(), SortByAverageAbsoluteDifference);
  //std::cout << "Blurred score for pair 0: " << candidatePairs[0].GetAverageAbsoluteDifference() << std::endl;
  candidatePairs.InvalidateAll();
  
  std::vector<float> blurredScores(candidatePairs.size());
  for(unsigned int i = 0; i < candidatePairs.size(); ++i)
    {
    blurredScores[i] = candidatePairs[i].GetAverageAbsoluteDifference();
    }

  // Create a temporary image to fill for now, but we might not actually end up filling this patch.
  FloatVectorImageType::Pointer tempImage = FloatVectorImageType::New();
  Helpers::DeepCopy<FloatVectorImageType>(this->CurrentOutputImage, tempImage);
  // Fill the detailed image hole with a part of the blurred image
  Helpers::CopySourcePatchIntoHoleOfTargetRegion<FloatVectorImageType>(this->BlurredImage, tempImage, this->CurrentMask, candidatePairs[0].SourcePatch.Region, candidatePairs[0].TargetPatch.Region);

  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(tempImage);
  this->PatchCompare->SetMask(this->CurrentMask);
  this->PatchCompare->ComputeAllSourceAndTargetDifferences();

  //std::cout << "Detailed score for pair 0: " << candidatePairs[0].GetAverageAbsoluteDifference() << std::endl;
  
  for(unsigned int i = 0; i < candidatePairs.size(); ++i)
    {
    candidatePairs[i].SetAverageAbsoluteDifference(blurredScores[i] + candidatePairs[i].GetAverageAbsoluteDifference());
    }

  std::cout << "Total score for pair 0: " << candidatePairs[0].GetAverageAbsoluteDifference() << std::endl;
  std::sort(candidatePairs.begin(), candidatePairs.end(), SortByAverageAbsoluteDifference);

  // Return the result by reference.
  bestPatchPair = candidatePairs[0];
  
  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatchScaleConsistent()." << std::endl;
  LeaveFunction("FindBestPatchScaleConsistent()");
}


void CriminisiInpainting::FindBestPatch(CandidatePairs& candidatePairs, PatchPair& bestPatchPair)
{
  EnterFunction("FindBestPatch()");

  //std::cout << "FindBestPatch: There are " << candidatePairs.size() << " candidate pairs." << std::endl;
  this->PatchCompare->SetPairs(&candidatePairs);
  this->PatchCompare->SetImage(this->CompareImage);
  this->PatchCompare->SetMask(this->CurrentMask);
  this->PatchCompare->ComputeAllSourceDifferences();
  
  //std::cout << "FindBestPatch: Finished ComputeAllSourceDifferences()" << std::endl;
  
  //std::sort(candidatePairs.begin(), candidatePairs.end(), SortByAverageAbsoluteDifference);
  //std::sort(candidatePairs.begin(), candidatePairs.end(), SortByDepthAndColor);
  std::sort(candidatePairs.begin(), candidatePairs.end(), PatchSortFunction);
  
  //std::cout << "Finished sorting " << candidatePairs.size() << " patches." << std::endl;
  
  // Return the result by reference.
  bestPatchPair = candidatePairs[0];
  
  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatch()." << std::endl;
  LeaveFunction("FindBestPatch()");
}

void CriminisiInpainting::FindBestPatchLookAhead(PatchPair& bestPatchPair)
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
  Helpers::DeepCopy<FloatScalarImageType>(this->PriorityImage, modifiedPriorityImage);
  
  UnsignedCharScalarImageType::Pointer modifiedBoundaryImage = UnsignedCharScalarImageType::New();
  Helpers::DeepCopy<UnsignedCharScalarImageType>(this->BoundaryImage, modifiedBoundaryImage);

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

    // If there are no boundary pixels, we can't find any more look ahead patches.
    if(Helpers::CountNonZeroPixels<UnsignedCharScalarImageType>(modifiedBoundaryImage) == 0)
      {
      std::cerr << "There are no boundary pixels!" << std::endl;
      break;
      }

    float highestPriority = 0;
    itk::Index<2> pixelToFill = FindHighestValueOnBoundary(modifiedPriorityImage, highestPriority, modifiedBoundaryImage);

    if(!Helpers::HasHoleNeighbor(pixelToFill, this->CurrentMask))
      {
      std::cerr << "pixelToFill " << pixelToFill << " does not have a hole neighbor - something is wrong!" << std::endl;
      std::cerr << "Mask value " << static_cast<unsigned int>(this->CurrentMask->GetPixel(pixelToFill)) << std::endl;
      std::cerr << "Boundary value " << static_cast<unsigned int>(this->BoundaryImage->GetPixel(pixelToFill)) << std::endl;
      exit(-1);
      }

    itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
    Patch targetPatch(targetRegion);

    CandidatePairs candidatePairs(targetPatch);
    candidatePairs.AddPairsFromPatches(this->SourcePatches);
    candidatePairs.Priority = highestPriority;
    
    PatchPair currentLookAheadBestPatchPair;
    //FindBestPatchScaleConsistent(candidatePairs, currentLookAheadBestPatchPair);
    FindBestPatch(candidatePairs, currentLookAheadBestPatchPair);
    
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
  
//   unsigned int bestForwardLookId = 0;
//   unsigned int bestSourcePatchId = 0;
//   ComputeMinimumBoundaryGradientChange(bestForwardLookId, bestSourcePatchId);
  
  unsigned int bestForwardLookId = ComputeMinimumScoreLookAhead();
  unsigned int bestSourcePatchId = 0;
    
  // Return the result by reference.
  bestPatchPair = this->PotentialCandidatePairs[bestForwardLookId][bestSourcePatchId];
  //std::cout << "Best pair found to be " << bestForwardLookId << " " << bestSourcePatchId << std::endl;
  
  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end." << std::endl;
  LeaveFunction("FindBestPatchLookAhead()");
}

unsigned int CriminisiInpainting::ComputeMinimumScoreLookAhead()
{
  EnterFunction("ComputeMinimumScoreLookAhead()");
  // Choose the look ahead with the lowest score to actually fill rather than simply returning the best source patch of the first look ahead target patch.
  float lowestScore = std::numeric_limits< float >::max();
  unsigned int lowestLookAhead = 0;
  for(unsigned int i = 0; i < this->PotentialCandidatePairs.size(); ++i)
    {
    if(this->PotentialCandidatePairs[i][0].GetAverageAbsoluteDifference() < lowestScore)
      {
      lowestScore = this->PotentialCandidatePairs[i][0].GetAverageAbsoluteDifference();
      lowestLookAhead = i;
      }
    }
  LeaveFunction("ComputeMinimumScoreLookAhead()");
  return lowestLookAhead;
}

void CriminisiInpainting::ComputeMinimumBoundaryGradientChange(unsigned int& bestForwardLookId, unsigned int& bestSourcePatchId)
{
  EnterFunction("ComputeMinimumBoundaryGradientChange()");
  // For the top N patches, compute the continuation difference by comparing the gradient at source side boundary pixels before and after filling.
  float lowestScore = std::numeric_limits< float >::max();
  
  itk::Index<2> zeroIndex;
  zeroIndex.Fill(0);
  itk::ImageRegion<2> outputRegion(zeroIndex, this->PotentialCandidatePairs[0][0].SourcePatch.Region.GetSize());
  
//   FloatScalarImageType::Pointer luminancePatch = FloatScalarImageType::New();
//   luminancePatch->SetRegions(outputRegion);
//   luminancePatch->SetNumberOfComponentsPerPixel(this->CurrentOutputImage->GetNumberOfComponentsPerPixel());
//   luminancePatch->Allocate();
  
  FloatVectorImageType::Pointer patch = FloatVectorImageType::New();
  patch->SetRegions(outputRegion);
  patch->SetNumberOfComponentsPerPixel(this->CurrentOutputImage->GetNumberOfComponentsPerPixel());
  patch->Allocate();
  
  FloatVector2ImageType::PixelType zeroVector;
  zeroVector.Fill(0);
  
  FloatVector2ImageType::Pointer preFillGradient = FloatVector2ImageType::New();
  preFillGradient->SetRegions(outputRegion);
  preFillGradient->Allocate();
  preFillGradient->FillBuffer(zeroVector);
  
  FloatVector2ImageType::Pointer postFillGradient = FloatVector2ImageType::New();
  postFillGradient->SetRegions(outputRegion);
  postFillGradient->Allocate();
  postFillGradient->FillBuffer(zeroVector);
  
  // Create an entirely unmasked Mask
  Mask::Pointer noMask = Mask::New();
  noMask->SetRegions(outputRegion);
  noMask->Allocate();
  
  itk::ImageRegionIterator<Mask> noMaskIterator(noMask, noMask->GetLargestPossibleRegion());

  while(!noMaskIterator.IsAtEnd())
    {
    noMaskIterator.Set(noMask->GetValidValue());
    ++noMaskIterator;
    }
  
  for(unsigned int forwardLookId = 0; forwardLookId < this->PotentialCandidatePairs.size(); ++forwardLookId)
    {
    std::cout << "Computing boundary gradient difference for forward look set " << forwardLookId << std::endl;
    // The boundary only need to be computed once for every forward look set
    std::vector<itk::Index<2> > boundaryPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage, this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region);
    if(boundaryPixels.size() < 1)
      {
      std::cerr << "There must be at least 1 boundary pixel!" << std::endl;
      exit(-1);
      }
      
    itk::Offset<2> patchOffset = this->CurrentMask->GetLargestPossibleRegion().GetIndex() - this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region.GetIndex();
    for(unsigned int boundaryPixelId = 0; boundaryPixelId < boundaryPixels.size(); ++boundaryPixelId)
      {
      boundaryPixels[boundaryPixelId] += patchOffset;
      }
      
    // Get the current mask
    typedef itk::RegionOfInterestImageFilter<Mask,Mask> ExtractFilterType;
    typename ExtractFilterType::Pointer extractMaskFilter = ExtractFilterType::New();
    extractMaskFilter->SetRegionOfInterest(this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region);
    extractMaskFilter->SetInput(this->CurrentMask);
    extractMaskFilter->Update();
  
    unsigned int maxNumberToInspect = 100u;
    unsigned int numberOfSourcePatchesToInspect = std::min(maxNumberToInspect, this->PotentialCandidatePairs[forwardLookId].size());
    for(unsigned int sourcePatchId = 0; sourcePatchId < numberOfSourcePatchesToInspect; ++sourcePatchId)
      {
      Helpers::CreatePatchImage<FloatVectorImageType>(this->CurrentOutputImage, this->PotentialCandidatePairs[forwardLookId][sourcePatchId].SourcePatch.Region, this->PotentialCandidatePairs[forwardLookId].TargetPatch.Region, this->CurrentMask, patch);

      float sumOfComponentErrors = 0.0f;
      for(unsigned int component = 0; component < this->CurrentOutputImage->GetNumberOfComponentsPerPixel(); ++component)
	{
// 	typedef itk::VectorImageToImageAdaptor<float, 2> ImageAdaptorType;
// 	ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
// 	adaptor->SetExtractComponentIndex(component);
// 	adaptor->SetImage(patch);
// 	
        typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
	IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
	indexSelectionFilter->SetIndex(component);
	indexSelectionFilter->SetInput(patch);
	indexSelectionFilter->Update();
	
	Helpers::SetImageToConstant<FloatVector2ImageType>(preFillGradient, zeroVector);
	Helpers::SetImageToConstant<FloatVector2ImageType>(postFillGradient, zeroVector);
	//float averageError = ComputeAverageGradientChange<ImageAdaptorType>(adaptor, preFillGradient, postFillGradient, extractMaskFilter->GetOutput(), noMask, boundaryPixels);
	float averageError = ComputeAverageGradientChange<FloatScalarImageType>(indexSelectionFilter->GetOutput(), preFillGradient, postFillGradient, extractMaskFilter->GetOutput(), noMask, boundaryPixels);
	
	sumOfComponentErrors += averageError;

	} // end component loop
      
      
      this->PotentialCandidatePairs[forwardLookId][sourcePatchId].SetBoundaryGradientDifference(sumOfComponentErrors);
      if(sumOfComponentErrors < lowestScore)
	{
	lowestScore = sumOfComponentErrors;
	bestForwardLookId = forwardLookId;
	bestSourcePatchId = sourcePatchId;
      
	Helpers::Write2DVectorImage(preFillGradient, "Debug/BestPrefill.mha");
	
	Helpers::Write2DVectorImage(postFillGradient, "Debug/BestPostfill.mha");
	
	Helpers::WriteVectorImageAsRGB(patch, "Debug/BestPatch.mha");
	}
      } // end source patch loop
    } // end forward look set loop
  LeaveFunction("ComputeMinimumBoundaryGradientChange()");
}

void CriminisiInpainting::Inpaint()
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


bool CriminisiInpainting::HasMoreToInpaint()
{
  EnterFunction("HasMoreToInpaint()");
  try
  {
    
    Helpers::WriteImageConditional<Mask>(this->CurrentMask, "Debug/HasMoreToInpaint.input.png", this->DebugImages);
    
    itk::ImageRegionIterator<Mask> maskIterator(this->CurrentMask, this->CurrentMask->GetLargestPossibleRegion());

    while(!maskIterator.IsAtEnd())
      {
      if(this->CurrentMask->IsHole(maskIterator.GetIndex()))
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

void CriminisiInpainting::FindBoundary()
{
  EnterFunction("FindBoundary()");
  try
  {
    // Compute the "outer" boundary of the region to fill. That is, we want the boundary pixels to be in the source region.

    Helpers::WriteImageConditional<Mask>(this->CurrentMask, "Debug/FindBoundary.CurrentMask.mha", this->DebugImages);
    Helpers::WriteImageConditional<Mask>(this->CurrentMask, "Debug/FindBoundary.CurrentMask.png", this->DebugImages);

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

    Helpers::WriteImageConditional<Mask>(holeOnly, "Debug/FindBoundary.HoleOnly.mha", this->DebugImages);
    Helpers::WriteImageConditional<Mask>(holeOnly, "Debug/FindBoundary.HoleOnly.png", this->DebugImages);
      
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

    Helpers::WriteImageConditional<Mask>(binaryContourFilter->GetOutput(), "Debug/FindBoundary.Boundary.mha", this->DebugImages);
    Helpers::WriteImageConditional<Mask>(binaryContourFilter->GetOutput(), "Debug/FindBoundary.Boundary.png", this->DebugImages);

    // Since we want to interpret non-zero pixels as boundary pixels, we must invert the image.
    typedef itk::InvertIntensityImageFilter <Mask> InvertIntensityImageFilterType;
    InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
    invertIntensityFilter->SetInput(binaryContourFilter->GetOutput());
    invertIntensityFilter->SetMaximum(255);
    invertIntensityFilter->Update();
    
    //this->BoundaryImage = binaryContourFilter->GetOutput();
    //this->BoundaryImage->Graft(binaryContourFilter->GetOutput());
    Helpers::DeepCopy<UnsignedCharScalarImageType>(invertIntensityFilter->GetOutput(), this->BoundaryImage);

    Helpers::WriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/FindBoundary.BoundaryImage.mha", this->DebugImages);
    LeaveFunction("FindBoundary()");
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
  EnterFunction("UpdateMask()");
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
    LeaveFunction("UpdateMask()");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateMask!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void CriminisiInpainting::ComputeBoundaryNormals(const float blurVariance)
{
  EnterFunction("ComputeBoundaryNormals()");
  try
  {
    // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

    Helpers::WriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/ComputeBoundaryNormals.BoundaryImage.mha", this->DebugImages);
    Helpers::WriteImageConditional<Mask>(this->CurrentMask, "Debug/ComputeBoundaryNormals.CurrentMask.mha", this->DebugImages);
      
    // Blur the mask
    typedef itk::DiscreteGaussianImageFilter< Mask, FloatScalarImageType >  BlurFilterType;
    BlurFilterType::Pointer gaussianFilter = BlurFilterType::New();
    gaussianFilter->SetInput(this->CurrentMask);
    gaussianFilter->SetVariance(blurVariance);
    gaussianFilter->Update();

    Helpers::WriteImageConditional<FloatScalarImageType>(gaussianFilter->GetOutput(), "Debug/ComputeBoundaryNormals.BlurredMask.mha", this->DebugImages);

    // Compute the gradient of the blurred mask
    typedef itk::GradientImageFilter< FloatScalarImageType, float, float>  GradientFilterType;
    GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
    gradientFilter->SetInput(gaussianFilter->GetOutput());
    gradientFilter->Update();

    Helpers::WriteImageConditional<FloatVector2ImageType>(gradientFilter->GetOutput(), "Debug/ComputeBoundaryNormals.BlurredMaskGradient.mha", this->DebugImages);

    // Only keep the normals at the boundary
    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput(gradientFilter->GetOutput());
    maskFilter->SetMaskImage(this->BoundaryImage);
    maskFilter->Update();

    Helpers::WriteImageConditional<FloatVector2ImageType>(maskFilter->GetOutput(), "Debug/ComputeBoundaryNormals.BoundaryNormalsUnnormalized.mha", this->DebugImages);

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

    Helpers::WriteImageConditional<FloatVector2ImageType>(this->BoundaryNormals, "Debug/ComputeBoundaryNormals.BoundaryNormals.mha", this->DebugImages);
    LeaveFunction("ComputeBoundaryNormals()");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeBoundaryNormals!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

itk::Index<2> CriminisiInpainting::FindHighestValueOnBoundary(const FloatScalarImageType::Pointer image, float& maxValue, UnsignedCharScalarImageType::Pointer boundaryImage)
{
  EnterFunction("FindHighestValueOnBoundary()");
  // Return the location of the highest pixel in 'image' out of the non-zero pixels in 'boundaryImage'. Return the value of that pixel by reference.
  try
  {
    // Explicity find the maximum on the boundary
    maxValue = 0.0f; // priorities are non-negative, so anything better than 0 will win
    
    std::vector<itk::Index<2> > boundaryPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(boundaryImage);
    
    if(boundaryPixels.size() <= 0)
      {
      std::cerr << "FindHighestValueOnBoundary(): No boundary pixels!" << std::endl;
      exit(-1);
      }

    itk::Index<2> locationOfMaxValue = boundaryPixels[0];
    
    for(unsigned int i = 0; i < boundaryPixels.size(); ++i)
      {
      if(image->GetPixel(boundaryPixels[i]) > maxValue)
	{
	maxValue = image->GetPixel(boundaryPixels[i]);
	locationOfMaxValue = boundaryPixels[i];
	}
      }
    DebugMessage<float>("Highest value: ", maxValue);
    LeaveFunction("FindHighestValueOnBoundary()");
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
  EnterFunction("ComputeAllPriorities()");
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
    LeaveFunction("ComputeAllPriorities()");
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
  // The difference between this funciton and Criminisi's original data term computation (ComputeDataTermCriminisi)
  // is that we claim there is no reason to penalize the priority of linear structures that don't have a perpendicular incident
  // angle with the boundary. Of course, we don't want to continue structures that are almost parallel with the boundary, but above
  // a threshold, the strength of the isophote should be more important than the angle of incidence.
  try
  {
    FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
    FloatVector2Type boundaryNormal = this->BoundaryNormals->GetPixel(queryPixel);

    DebugMessage<FloatVector2Type>("Isophote: ", isophote);
    DebugMessage<FloatVector2Type>("Boundary normal: ", boundaryNormal);
    // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

    vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

    vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

    float dataTerm = 0.0f;

    float angleBetween = Helpers::AngleBetween(isophote, boundaryNormal);
    if(angleBetween < 20)
      {
      float projectionMagnitude = isophote.GetNorm() * cos(angleBetween);
      
      dataTerm = projectionMagnitude;
      }
    else
    {
      dataTerm = isophote.GetNorm();
    }

    return dataTerm;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

float CriminisiInpainting::ComputeDataTermCriminisi(const itk::Index<2>& queryPixel)
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
  
unsigned int CriminisiInpainting::GetNumberOfCompletedIterations()
{
  return this->NumberOfCompletedIterations;
}

bool CriminisiInpainting::GetAdjacentBoundaryPixel(const itk::Index<2>& targetPatchSourceSideBoundaryPixel, const PatchPair& patchPair,
                                                   itk::Index<2>& sourcePatchTargetSideBoundaryPixel)
{
  if(this->CurrentMask->IsHole(targetPatchSourceSideBoundaryPixel) || !patchPair.TargetPatch.Region.IsInside(targetPatchSourceSideBoundaryPixel))
    {
    std::cerr << "Error: The input boundary pixel must be on the valid side of the boundary (not in the hole)!" << std::endl;
    exit(-1);
    }
  
  FloatVector2Type sourceSideIsophote = this->IsophoteImage->GetPixel(targetPatchSourceSideBoundaryPixel);
    
  itk::Index<2> pixelAcrossBoundary = Helpers::GetNextPixelAlongVector(targetPatchSourceSideBoundaryPixel, sourceSideIsophote);
  
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
    pixelAcrossBoundary = Helpers::GetNextPixelAlongVector(targetPatchSourceSideBoundaryPixel, sourceSideIsophote);
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

  // Determine the position of the corresponding pixel in the source patch and return by reference.
  sourcePatchTargetSideBoundaryPixel = patchPair.SourcePatch.Region.GetIndex() + intraPatchOffset;

  return valid;
}

FloatVector2Type CriminisiInpainting::ComputeAverageIsophoteSourcePatch(const itk::Index<2>& sourcePatchPixel, const PatchPair& patchPair)
{
  // This function computes the average isophote of the pixels around 'pixel' in the target side of the source patch (pixels that will end up filling the hole).
  // The input 'pixel' is expected to be on the target side of the boundary in the source patch.

  // The target patch is the only patch in which the hole/boundary is actually defined, so computations must take place in that frame.
  itk::Index<2> targetPatchPixel = sourcePatchPixel + patchPair.GetSourceToTargetOffset();
  
  itk::ImageRegion<2> smallTargetPatch = Helpers::GetRegionInRadiusAroundPixel(targetPatchPixel, 1);
  smallTargetPatch.Crop(patchPair.TargetPatch.Region);
  
  // Get the pixels in the hole of the target patch.
  std::vector<itk::Index<2> > holeTargetPixels = this->CurrentMask->GetHolePixelsInRegion(smallTargetPatch);

  // We actually want the hole pixels in the source region, not the target region, so shift them.
  std::vector<itk::Index<2> > holeSourcePixels;
  itk::Offset<2> shiftAmount = patchPair.GetTargetToSourceOffset();
  for(unsigned int i = 0; i < holeTargetPixels.size(); ++i)
    {
    itk::Index<2> shiftedPixel = holeTargetPixels[i] + shiftAmount;

    holeSourcePixels.push_back(shiftedPixel);
    }

  std::vector<FloatVector2Type> sourceIsophotes;
  for(unsigned int i = 0; i < holeSourcePixels.size(); ++i)
    {
    sourceIsophotes.push_back(this->IsophoteImage->GetPixel(holeSourcePixels[i]));
    }

  FloatVector2Type averageSourceIsophote = Helpers::AverageVectors(sourceIsophotes);
  return averageSourceIsophote;
}

FloatVector2Type CriminisiInpainting::ComputeAverageIsophoteTargetPatch(const itk::Index<2>& pixel, const PatchPair& patchPair)
{
  // This function computes the average isophote of the pixels around 'pixel' in the source side of the target patch.

  itk::ImageRegion<2> smallTargetPatch = Helpers::GetRegionInRadiusAroundPixel(pixel, 1);
  smallTargetPatch.Crop(patchPair.TargetPatch.Region);

  // Get the pixels in the valid region and in the hole of the target patch.
  std::vector<itk::Index<2> > validTargetPixels = this->CurrentMask->GetValidPixelsInRegion(smallTargetPatch);

  std::vector<FloatVector2Type> targetIsophotes;
  for(unsigned int i = 0; i < validTargetPixels.size(); ++i)
    {
    targetIsophotes.push_back(this->IsophoteImage->GetPixel(validTargetPixels[i]));
    }

  FloatVector2Type averageTargetIsophote = Helpers::AverageVectors(targetIsophotes);

  return averageTargetIsophote;
}


float CriminisiInpainting::ComputeIsophoteAngleDifference(const FloatVector2Type& v1, const FloatVector2Type& v2)
{
  //std::cout << "ComputeIsophoteAngleDifference()" << std::endl;
  // Compute the isophote difference.
  float isophoteDifference = Helpers::AngleBetween(v1, v2);
  
  float isophoteDifferenceNormalized = isophoteDifference/3.14159; // The maximum angle between vectors is pi, so this produces a score between 0 and 1.
  DebugMessage<float>("isophoteDifferenceNormalized: ", isophoteDifferenceNormalized);

  //std::cout << "Leave ComputeIsophoteDifference()" << std::endl;
  return isophoteDifferenceNormalized;
}

float CriminisiInpainting::ComputeIsophoteStrengthDifference(const FloatVector2Type& v1, const FloatVector2Type& v2)
{
  //std::cout << "ComputeIsophoteStrengthDifference()" << std::endl;
  // Compute the isophote difference.
  float isophoteDifference = fabs(v1.GetNorm() - v2.GetNorm());

  return isophoteDifference;
}

float CriminisiInpainting::ComputeNormalizedSquaredPixelDifference(const itk::Index<2>& pixel1, const itk::Index<2>& pixel2)
{
  // Compute the pixel difference.
      
  FloatVectorImageType::PixelType value1 = this->CompareImage->GetPixel(pixel1);
  FloatVectorImageType::PixelType value2 = this->CompareImage->GetPixel(pixel2);
  DebugMessage<FloatVectorImageType::PixelType>("value1 ", value1);
  DebugMessage<FloatVectorImageType::PixelType>("value2 ", value2);
  
  float pixelSquaredDifference = FullSquaredPixelDifference::Difference(this->CompareImage->GetPixel(pixel1), this->CompareImage->GetPixel(pixel2), this->CompareImage->GetNumberOfComponentsPerPixel());
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
  EnterFunction("ComputeAllContinuationDifferences()");
  
  // Naively, we could just call ComputeTotalContinuationDifference on each patch pair for a forward looking set. However, this
  // would recalculate the boundary for every pair. This is a lot of extra work because the boundary does not change from source patch to source patch.
  
  // Identify border pixels on the source side of the boundary.
  std::vector<itk::Index<2> > borderPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage, candidatePairs.TargetPatch.Region);
  
  //for(unsigned int sourcePatchId = 0; sourcePatchId < this->SourcePatches.size(); ++sourcePatchId)
  for(unsigned int sourcePatchId = 0; sourcePatchId < candidatePairs.size(); ++sourcePatchId)
    {
    // Only compute if the values are not already computed.
    if(candidatePairs[sourcePatchId].IsValidBoundaryIsophoteAngleDifference() &&
       candidatePairs[sourcePatchId].IsValidBoundaryIsophoteStrengthDifference() &&
       candidatePairs[sourcePatchId].IsValidBoundaryPixelDifference())
       //candidatePairs[sourcePatchId].IsValidSSD()) // Don't check this, it is not related to the continuation difference
      {
      continue;
      }
    float totalPixelDifference = 0.0f;
    float totalIsophoteAngleDifference = 0.0f;
    float totalIsophoteStrengthDifference = 0.0f;
    unsigned int numberUsed = 0;
    for(unsigned int pixelId = 0; pixelId < borderPixels.size(); ++pixelId)
      {
      itk::Index<2> targetRegionSourceSideBoundaryPixel = borderPixels[pixelId];
      itk::Index<2> sourceRegionTargetSideBoundaryPixel;
      bool valid = GetAdjacentBoundaryPixel(targetRegionSourceSideBoundaryPixel, candidatePairs[sourcePatchId], sourceRegionTargetSideBoundaryPixel);
      if(!valid)
	{
	continue;
	}
      numberUsed++;

      // Pixel difference
      float normalizedSquaredPixelDifference = ComputeNormalizedSquaredPixelDifference(targetRegionSourceSideBoundaryPixel, sourceRegionTargetSideBoundaryPixel);
      totalPixelDifference += normalizedSquaredPixelDifference;
      DebugMessage<float>("ComputeAllContinuationDifferences::normalizedSquaredPixelDifference ", normalizedSquaredPixelDifference);

      // Isophote differences
      FloatVector2Type averageSourceIsophote = ComputeAverageIsophoteSourcePatch(sourceRegionTargetSideBoundaryPixel, candidatePairs[sourcePatchId]);
      FloatVector2Type averageTargetIsophote = ComputeAverageIsophoteTargetPatch(targetRegionSourceSideBoundaryPixel, candidatePairs[sourcePatchId]);
      
      float isophoteAngleDifference = ComputeIsophoteAngleDifference(averageSourceIsophote, averageTargetIsophote);
      totalIsophoteAngleDifference += isophoteAngleDifference;
      
      float isophoteStrengthDifference = ComputeIsophoteStrengthDifference(averageSourceIsophote, averageTargetIsophote);
      totalIsophoteStrengthDifference += isophoteStrengthDifference;

      } // end loop over pixels

    DebugMessage<unsigned int>("numberUsed ", numberUsed);
    DebugMessage<unsigned int>("Out of ", borderPixels.size());

    if(numberUsed == 0)
      {
      std::cout << "Warning: no pixels were used in ComputeAllContinuationDifferences()" << std::endl;
      numberUsed = 1; // Set this to 1 to avoid divide by zero.
      }

    float averagePixelDifference = totalPixelDifference / static_cast<float>(numberUsed);
    DebugMessage<float>("averagePixelDifference ", averagePixelDifference);

    float averageIsophoteAngleDifference = totalIsophoteAngleDifference / static_cast<float>(numberUsed);
    float averageIsophoteStrengthDifference = totalIsophoteStrengthDifference / static_cast<float>(numberUsed);
    candidatePairs[sourcePatchId].SetBoundaryPixelDifference(averagePixelDifference);
    candidatePairs[sourcePatchId].SetBoundaryIsophoteAngleDifference(averageIsophoteAngleDifference);
    candidatePairs[sourcePatchId].SetBoundaryIsophoteStrengthDifference(averageIsophoteStrengthDifference);

    } // end loop over pairs

  LeaveFunction("ComputeAllContinuationDifferences()");
}

std::vector<CandidatePairs>& CriminisiInpainting::GetPotentialCandidatePairsReference()
{
  // Return a reference to the whole set of forward look pairs.
  return PotentialCandidatePairs;
}

void CriminisiInpainting::SetCompareToOriginal()
{
  this->CompareImage = this->CurrentOutputImage;
}
  
void CriminisiInpainting::SetCompareToBlurred()
{
  this->CompareImage = this->BlurredImage;
}

void CriminisiInpainting::SetCompareToCIELAB()
{
  this->CompareImage = this->CIELabImage;
}

void CriminisiInpainting::SetPatchCompare(SelfPatchCompare* patchCompare)
{
  delete this->PatchCompare;
  this->PatchCompare = patchCompare;
}

void CriminisiInpainting::BlurImage()
{
  EnterFunction("BlurImage()");
  Helpers::BlurAllChannels<FloatVectorImageType>(this->OriginalImage, this->BlurredImage, 10);
  Helpers::WriteImageConditional<FloatVectorImageType>(this->BlurredImage, "Debug/Initialize.BlurredImage.mha", this->DebugImages);
  Helpers::WriteVectorImageAsRGB(this->BlurredImage, "Debug/Initialize.BlurredImageRGB.mha");
  LeaveFunction("BlurImage()");
}

void CriminisiInpainting::SetDebugFunctionEnterLeave(const bool value)
{
  this->DebugFunctionEnterLeave = value;
}
