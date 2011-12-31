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
#include "Canvas.h"

// Custom
#include "Helpers.h"
#include "HelpersQt.h"

// VTK
#include <vtkImageSlice.h>
#include <vtkRenderer.h>

VTKCanvas::VTKCanvas(vtkRenderer* const renderer) : Renderer(renderer)
{
  this->ImageLayer.ImageSlice->SetPickable(false);
  this->MaskLayer.ImageSlice->SetPickable(false);
  this->UsedPatchPairLayer.ImageSlice->SetPickable(false);
  this->SourcePatchLayer.ImageSlice->SetPickable(false);
  this->ForwardLookPatchLayer.ImageSlice->SetPickable(false);

  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);
  this->Renderer->AddViewProp(this->MaskLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedPatchPairLayer.ImageSlice);
  this->Renderer->AddViewProp(this->SourcePatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->ForwardLookPatchLayer.ImageSlice);
}

void VTKCanvas::DisplayMask(const Mask* const mask)
{
  //vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  //Helpers::ITKScalarImageToScaledVTKImage<Mask>(this->IntermediateImages[this->IterationToDisplay].MaskImage, temp);
  //Helpers::MakeValidPixelsTransparent(temp, this->MaskLayer.ImageData, 0); // Set the zero pixels of the mask to transparent

  mask->MakeVTKImage(this->MaskLayer.ImageData, QColor(Qt::white), this->Colors.Hole, false, true); // (..., holeTransparent, validTransparent);
}

void VTKCanvas::DisplayImage(const FloatVectorImageType* const image)
{
  ITKVectorImageToVTKImage(image, this->ImageLayer.ImageData, this->ImageDisplayStyle);
}

// Convert a vector ITK image to a VTK image for display
void VTKCanvas::ITKVectorImageToVTKImage(const FloatVectorImageType* const image, vtkImageData* outputImage, const DisplayStyle& style)
{
  switch(style.Style)
    {
    case DisplayStyle::COLOR:
      Helpers::ITKImageToVTKRGBImage(image, outputImage);
      break;
    case DisplayStyle::MAGNITUDE:
      Helpers::ITKImageToVTKMagnitudeImage(image, outputImage);
      break;
    case DisplayStyle::CHANNEL:
      Helpers::ITKImageChannelToVTKImage(image, style.Channel, outputImage);
      break;
    default:
      std::cerr << "No valid style to display!" << std::endl;
      return;
    }

  outputImage->Modified();
}

void VTKCanvas::HighlightUsedPatchPair(const PatchPair& patchPair)
{
  std::vector<Patch> patches;
  patches.push_back(*(patchPair.GetSourcePatch()));
  patches.push_back(*(patchPair.GetTargetPatch()));

  HighlightPatches(patches, this->Colors.UsedSourcePatch, this->UsedPatchPairLayer.ImageData);
  // TODO: These should be different colors.
}

void VTKCanvas::HighlightForwardLookPatches(const std::vector<Patch>& patches)
{
  HighlightPatches(patches, this->Colors.ForwardLookPatch, this->UsedPatchPairLayer.ImageData);
}

void VTKCanvas::HighlightSourcePatches(const std::vector<Patch>& patches)
{
  HighlightPatches(patches, this->Colors.SourcePatch, this->UsedPatchPairLayer.ImageData);
}

void VTKCanvas::HighlightPatches(const std::vector<Patch>& patches, const QColor& color, vtkImageData* outputImage)
{
  // Delete any current highlight patches. We want to delete these (if they exist) no matter what because
  // then they won't be displayed if the box is not checked (they will respect the check box).
  Helpers::BlankImage(outputImage);

  //unsigned char centerPixelColor[3];
  //HelpersQt::QColorToUCharColor(this->Colors.CenterPixel, centerPixelColor);

  for(unsigned int patchId = 0; patchId < patches.size(); ++patchId)
    {
    Patch currentPatch = patches[patchId];
    unsigned char borderColor[3];
    HelpersQt::QColorToUCharColor(color, borderColor);

    Helpers::BlankAndOutlineRegion(outputImage, currentPatch.GetRegion(), borderColor);

    //Helpers::SetRegionCenterPixel(outputImage, currentPatch.Region, centerPixelColor);
    }
}
