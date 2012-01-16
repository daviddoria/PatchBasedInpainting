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
#include "ItemDifferenceVisitor.h"
#include "MaskOperations.h"
#include "PatchDifferencePixelWiseSum.h"
#include "Priority.h"
#include "PrioritySearchHighest.h"
#include "Types.h"
#include "ValidPixelIterator.h"
#include "ValidRegionIterator.h"

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
  // If the user hasn't specified a priority function, use the simplest one.
  if(!this->PriorityFunction)
    {
    throw std::runtime_error("You must specify a Priority function to use!");
    }

  // If the user hasn't specified a priority function, use the simplest one.
  if(!this->ItemCreatorObject)
    {
    throw std::runtime_error("You must specify an ItemCreator to use!");
    }

  this->ItemImage = ItemImageType::New();
  this->ItemImage->SetRegions(this->FullImageRegion);
  this->ItemImage->Allocate();

  itk::ImageRegionIterator<ItemImageType> iterator(this->ItemImage, this->ItemImage->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    iterator.Set(NULL);
    ++iterator;
    }

  AddNewObjectsInRegion(this->MaskImage->GetLargestPossibleRegion());

  this->NumberOfCompletedIterations = 0;
}

template <typename TImage>
void PatchBasedInpainting<TImage>::AddNewObjectsInRegion(const itk::ImageRegion<2>& region)
{
  ValidRegionIterator validRegionIterator(this->MaskImage, region, this->PatchRadius[0]);

  for(ValidRegionIterator::ConstIterator iterator = validRegionIterator.begin(); iterator != validRegionIterator.end(); ++iterator)
    {
    itk::Index<2> centerPixel = ITKHelpers::GetRegionCenter(*iterator);
    Item* newItem = ItemCreatorObject->CreateItem(centerPixel);
    this->ItemImage->SetPixel(centerPixel, newItem);
    ++iterator;
    }
}

template <typename TImage>
itk::ImageRegion<2> PatchBasedInpainting<TImage>::FindBestMatch(const itk::Index<2>& targetPixel)
{
  //ItemDifferenceVisitor itemDifferenceVisitor(this->ItemImage->GetPixel(targetPixel), this->ItemDifferenceMap);
  assert(this->DifferenceVisitor);
  this->DifferenceVisitor->SetItemToCompare(this->ItemImage->GetPixel(targetPixel));
  this->DifferenceVisitor->SetDifferenceMap(this->ItemDifferenceMap);

  ValidPixelIterator<ItemImageType> validPixelIterator(this->ItemImage, this->ItemImage->GetLargestPossibleRegion());

  for(ValidPixelIterator<ItemImageType>::ConstIterator iterator = validPixelIterator.begin(); iterator != validPixelIterator.end(); ++iterator)
    {
    this->DifferenceVisitor->Visit(this->ItemImage->GetPixel(*iterator));
    ++iterator;
    }
}

template <typename TImage>
void PatchBasedInpainting<TImage>::SetDifferenceVisitor(ItemDifferenceVisitor* const differenceVisitor)
{
  this->DifferenceVisitor = std::shared_ptr<ItemDifferenceVisitor>(differenceVisitor);
}

template <typename TImage>
PatchPair<TImage> PatchBasedInpainting<TImage>::Iterate()
{
  itk::ImageRegion<2> targetRegion = DetermineRegionToFill();

  itk::ImageRegion<2> sourceRegion = FindBestMatch(ITKHelpers::GetRegionCenter(targetRegion));

  // Copy the patch. This is the actual inpainting step.
  ITKHelpers::CopySelfRegion(this->CurrentInpaintedImage, sourceRegion, targetRegion);

  // Update the mask
  ITKHelpers::CopySelfRegion(this->MaskImage, sourceRegion, targetRegion);

  // Update the priority function
  this->PriorityFunction->Update(targetRegion);

  this->NumberOfCompletedIterations++;

  AddNewObjectsInRegion(targetRegion);

  // TODO: This shouldn't be a PatchPair anymore because 'Patch' is now associated with an image.
  //PatchPair<TImage> patchPair(sourceRegion, targetRegion);
  //return usedPatchPair;
}

template <typename TImage>
itk::ImageRegion<2> PatchBasedInpainting<TImage>::DetermineRegionToFill()
{
  PrioritySearchHighest prioritySearchHighest;
  itk::Index<2> pixelToFill = prioritySearchHighest.FindHighestPriority(this->BoundaryPixels, this->PriorityFunction.get());

  itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(pixelToFill, this->PatchRadius[0]);

  return targetRegion;
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

  // If no pixels were holes, then we don't have any more to inpaint.
  return false;
}

template <typename TImage>
unsigned int PatchBasedInpainting<TImage>::GetNumberOfCompletedIterations()
{
  return this->NumberOfCompletedIterations;
}
