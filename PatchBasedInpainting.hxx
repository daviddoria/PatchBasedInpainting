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

#include "PatchBasedInpainting.h" // Appease syntax parser

// Custom
#include "Derivatives.h"
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "Item.h"
#include "MaskOperations.h"
#include "PatchDifferencePixelWiseSum.h"
#include "Priority.h"
#include "PrioritySearchHighest.h"
#include "Types.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "Helpers/ITKHelpers.h"

// STL
#include <iostream>

template <typename TImage>
PatchBasedInpainting<TImage>::PatchBasedInpainting(const TImage* const image, const Mask* const mask)
{
  EnterFunction("PatchBasedInpainting()");

  this->PatchRadius.Fill(3);

  // We don't want to modify the input images, so we copy them.
  this->MaskImage = Mask::New();
  this->MaskImage->DeepCopyFrom(mask);

  this->CurrentInpaintedImage = TImage::New();
  ITKHelpers::DeepCopy<TImage>(image, this->CurrentInpaintedImage);

  ColorImageInsideHole();

  this->FullImageRegion = image->GetLargestPossibleRegion();
  if(this->MaskImage->GetLargestPossibleRegion() != this->FullImageRegion)
    {
    std::stringstream ss;
    ss << "Mask and image size must match! Mask is " << this->MaskImage->GetLargestPossibleRegion().GetSize()
              << " while image is " << this->FullImageRegion << std::endl;
    throw std::runtime_error(ss.str());
    }

  // Set defaults
  this->NumberOfCompletedIterations = 0;

  this->PriorityFunction = NULL; // Can't initialize this here, must wait until the image and mask are opened
}

template <typename TImage>
TImage* PatchBasedInpainting<TImage>::GetCurrentOutputImage()
{
  return this->CurrentInpaintedImage;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::SetPatchRadius(const unsigned int radius)
{
  // Since this is the radius of the patch, there are no restrictions for the radius to be odd or even.
  this->PatchRadius.Fill(radius);
}

template <typename TImage>
unsigned int PatchBasedInpainting<TImage>::GetPatchRadius() const
{
  return this->PatchRadius[0];
}

template <typename TImage>
const itk::ImageRegion<2>& PatchBasedInpainting<TImage>::GetFullRegion() const
{
  return this->FullImageRegion;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::SetPriorityFunction(Priority* const priority)
{
  this->PriorityFunction = priority;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::ColorImageInsideHole()
{
  // Color the target image bright green inside the hole. This is helpful when watching the inpainting proceed, as you can clearly see
  // the region that is being filled.

  typename TImage::PixelType fillColor;
  fillColor.SetSize(this->CurrentInpaintedImage->GetNumberOfComponentsPerPixel());
  fillColor.Fill(0);
  fillColor[0] = 255;
  // We could use MaskImage->ApplyColorToImage here to use a predefined QColor, but this would introduce a dependency on Qt in the non-GUI part of the code.
  this->MaskImage->template ApplyToImage<TImage>(this->CurrentInpaintedImage, fillColor);
}

template <typename TImage>
void PatchBasedInpainting<TImage>::Initialize()
{
  typedef itk::Image<Item*, 2> ImageOfItems;
  ImageOfItems::Pointer itemImage = ImageOfItems::New();
  itemImage->SetRegions(this->FullImageRegion);
  itemImage->Allocate();

  itk::ImageRegionIterator<ImageOfItems> iterator(itemImage, itemImage->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    if(this->MaskImage->IsHole(iterator.GetIndex()))
      {
      iterator.Set(ItemCreatorObject->CreateItem(iterator.GetIndex()));
      }

    ++iterator;
    }

  // If the user hasn't specified a priority function, use the simplest one.
  if(!this->PriorityFunction)
    {
    throw std::runtime_error("You must specify a Priority function to use!");
    }

  this->NumberOfCompletedIterations = 0;

  HelpersOutput::WriteImageConditional<FloatVectorImageType>(this->CurrentInpaintedImage, "Debug/Initialize.CurrentOutputImage.mha", this->DebugImages);

  this->SourcePatches = new SourcePatchCollection<TImage>(this->MaskImage, this->PatchRadius[0]);

  // Clear the source patches, as additional patches are added each iteration. When we reset the inpainter, we want to start over from only patches that are
  // valid in the original mask.
  this->SourcePatches->Clear();
}

template <typename TImage>
PatchPair<TImage> PatchBasedInpainting<TImage>::Iterate()
{
  EnterFunction("Iterate()");

  typename SourcePatchCollection<TImage>::PatchContainer sourcePatches = this->SourcePatches->FindSourcePatchesInRegion(this->FullImageRegion);
  this->SourcePatches->AddPatches(sourcePatches);

  PatchPair<TImage> usedPatchPair = FindBestPatch();

  // Copy the patch. This is the actual inpainting step.
  // TODO: Change this to match new interfaces.
  //ImagesToUpdate.CopySelfPatchIntoHoleOfTargetRegion(this->MaskImage, usedPatchPair.GetSourcePatch()->GetRegion(), usedPatchPair.GetTargetPatch().GetRegion());
  std::cout << "Image size: " << this->CurrentInpaintedImage->GetLargestPossibleRegion().GetSize() << std::endl;

  this->PriorityFunction->Update(usedPatchPair.GetTargetPatch().GetRegion());

  this->NumberOfCompletedIterations++;

  DebugMessage<unsigned int>("Completed iteration: ", this->NumberOfCompletedIterations);

  LeaveFunction("Iterate()");
  return usedPatchPair;
}


template <typename TImage>
PatchPair<TImage> PatchBasedInpainting<TImage>::FindBestPatch()
{
  EnterFunction("PatchBasedInpainting::FindBestPatch()");

  float highestPriority = 0.0f;

  PrioritySearchHighest prioritySearchHighest;
  itk::Index<2> pixelToFill = prioritySearchHighest.FindHighestPriority(this->BoundaryPixels, this->PriorityFunction.get());

  itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);
  ImagePatch<TImage> targetPatch(this->CurrentInpaintedImage, targetRegion);
  CandidatePairs<TImage> candidatePairs(targetPatch);
  candidatePairs.AddSourcePatches(*(this->SourcePatches));
  candidatePairs.SetPriority(highestPriority);

  candidatePairs.Sort(PatchPairDifferences::AveragePixelDifference);

  //std::cout << "Finished sorting " << candidatePairs.size() << " patches." << std::endl;

  PatchPair<TImage> bestPatchPair = *(candidatePairs.begin());

  //std::cout << "There are " << this->SourcePatches.size() << " source patches at the end of FindBestPatch()." << std::endl;
  //LeaveFunction("PatchBasedInpainting::FindBestPatch()");
  return bestPatchPair;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::Inpaint()
{
  EnterFunction("Inpaint()");
  // This function is intended to be used by the command line version.
  // It will do the complete inpainting without updating any UI or the ability to stop before it is complete.

  // Start the procedure
  Initialize();

  this->NumberOfCompletedIterations = 0;
  while(HasMoreToInpaint())
    {
    Iterate();
    }
  //std::cout << "Finished inpainting." << std::endl;
  LeaveFunction("Inpaint()");

}

template <typename TImage>
bool PatchBasedInpainting<TImage>::HasMoreToInpaint()
{
  EnterFunction("HasMoreToInpaint()");

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

template <typename TImage>
bool PatchBasedInpainting<TImage>::IsValidPatch(const itk::Index<2>& queryPixel, const unsigned int radius)
{
  // This function checks if a patch is completely inside the image and not intersecting the mask

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, radius);
  return IsValidRegion(region);
}

template <typename TImage>
bool PatchBasedInpainting<TImage>::IsValidRegion(const itk::ImageRegion<2>& region)
{
  return this->MaskImage->IsValid(region);
}

template <typename TImage>
unsigned int PatchBasedInpainting<TImage>::GetNumberOfPixelsInPatch()
{
  return this->GetPatchSize()[0]*this->GetPatchSize()[1];
}

template <typename TImage>
itk::Size<2> PatchBasedInpainting<TImage>::GetPatchSize()
{
  itk::Size<2> patchSize;

  patchSize[0] = Helpers::SideLengthFromRadius(this->PatchRadius[0]);
  patchSize[1] = Helpers::SideLengthFromRadius(this->PatchRadius[1]);

  return patchSize;
}

template <typename TImage>
unsigned int PatchBasedInpainting<TImage>::GetNumberOfCompletedIterations()
{
  return this->NumberOfCompletedIterations;
}

template <typename TImage>
ITKImageCollection& PatchBasedInpainting<TImage>::GetImagesToUpdate()
{
  return this->ImagesToUpdate;
}
