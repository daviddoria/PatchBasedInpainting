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
#include "InpaintingIterationRecordViewer.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKVTKHelpers.h"
#include "Helpers/VTKHelpers.h"
#include "HelpersQt.h"

// VTK
#include <vtkImageSlice.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

InpaintingIterationRecordViewer::InpaintingIterationRecordViewer(vtkRenderer* const renderer) : Renderer(renderer)
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

void InpaintingIterationRecordViewer::DisplayRecord(const InpaintingIterationRecord& record)
{
  // TODO: important commented function calls
  this->RecordToDisplay = record;
  //HighlightUsedPatchPair();
  HighlightSourcePatches();
  HighlightForwardLookPatches();
  //DisplayImage();
  //DisplayMask();
}

DisplayStyle const& InpaintingIterationRecordViewer::GetImageDisplayStyle() const
{
  return this->ImageDisplayStyle;
}

DisplayStyle& InpaintingIterationRecordViewer::GetImageDisplayStyle()
{
  return this->ImageDisplayStyle;
}

ColorPalette const& InpaintingIterationRecordViewer::GetColorPalette() const
{
  return this->Colors;
}

void InpaintingIterationRecordViewer::DisplayMask(const Mask* const mask)
{
  //vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  //Helpers::ITKScalarImageToScaledVTKImage<Mask>(this->IntermediateImages[this->IterationToDisplay].MaskImage, temp);
  //Helpers::MakeValidPixelsTransparent(temp, this->MaskLayer.ImageData, 0); // Set the zero pixels of the mask to transparent

  mask->MakeVTKImage(this->MaskLayer.ImageData, QColor(Qt::white), this->Colors.Hole, false, true); // (..., holeTransparent, validTransparent);
}

void InpaintingIterationRecordViewer::DisplayImage(const FloatVectorImageType* const image)
{
  ITKVectorImageToVTKImage(image, this->ImageLayer.ImageData);
}

void InpaintingIterationRecordViewer::SetDisplayStyle(const DisplayStyle& style)
{
  this->ImageDisplayStyle = style;
}

void InpaintingIterationRecordViewer::SetDisplayState(const DisplayState& displayState)
{
  this->RecordDisplayState = displayState;
}

// Convert a vector ITK image to a VTK image for display
void InpaintingIterationRecordViewer::ITKVectorImageToVTKImage(const FloatVectorImageType* const image, vtkImageData* const outputImage)
{
  switch(this->ImageDisplayStyle.Style)
    {
    case DisplayStyle::COLOR:
      ITKVTKHelpers::ITKImageToVTKRGBImage(image, outputImage);
      break;
    case DisplayStyle::MAGNITUDE:
      ITKVTKHelpers::ITKImageToVTKMagnitudeImage(image, outputImage);
      break;
    case DisplayStyle::CHANNEL:
      ITKVTKHelpers::ITKImageChannelToVTKImage(image, this->ImageDisplayStyle.Channel, outputImage);
      break;
    default:
      std::cerr << "No valid style to display!" << std::endl;
      return;
    }

  outputImage->Modified();
}

void InpaintingIterationRecordViewer::HighlightUsedPatchPair(const PatchPair& patchPair)
{
  std::vector<Patch> patches;
  patches.push_back(*(patchPair.GetSourcePatch()));
  patches.push_back(patchPair.GetTargetPatch());

  HighlightPatches(patches, this->Colors.UsedSourcePatch, this->UsedPatchPairLayer.ImageData);
  // TODO: These should be different colors.
}

void InpaintingIterationRecordViewer::HighlightForwardLookPatches(const std::vector<Patch>& patches)
{
  HighlightPatches(patches, this->Colors.ForwardLookPatch, this->UsedPatchPairLayer.ImageData);
}

void InpaintingIterationRecordViewer::HighlightSourcePatches(const std::vector<Patch>& patches)
{
  HighlightPatches(patches, this->Colors.SourcePatch, this->UsedPatchPairLayer.ImageData);
}

void InpaintingIterationRecordViewer::HighlightPatches(const std::vector<Patch>& patches, const QColor& color, vtkImageData* const outputImage)
{
  // Delete any current highlight patches. We want to delete these (if they exist) no matter what because
  // then they won't be displayed if the box is not checked (they will respect the check box).
  VTKHelpers::ZeroImage(outputImage, 3);

  //unsigned char centerPixelColor[3];
  //HelpersQt::QColorToUCharColor(this->Colors.CenterPixel, centerPixelColor);

  for(unsigned int patchId = 0; patchId < patches.size(); ++patchId)
    {
    Patch currentPatch = patches[patchId];
    unsigned char borderColor[3];
    HelpersQt::QColorToUCharColor(color, borderColor);

    ITKVTKHelpers::BlankAndOutlineRegion(outputImage, currentPatch.GetRegion(), borderColor);

    //Helpers::SetRegionCenterPixel(outputImage, currentPatch.Region, centerPixelColor);
    }
}

void InpaintingIterationRecordViewer::HighlightSourcePatches()
{
  const CandidatePairs& candidatePairs = *(this->RecordToDisplay.PotentialPairSets[this->RecordDisplayState.ForwardLookId]);
  unsigned int numberToDisplay = std::min(candidatePairs.GetNumberOfSourcePatches(), this->DisplaySettings.NumberOfTopPatchesToDisplay);
}

void InpaintingIterationRecordViewer::SetSettings(const InpaintingDisplaySettings& settings)
{
  this->DisplaySettings = settings;
}

void InpaintingIterationRecordViewer::HighlightForwardLookPatches()
{
  // Get the candidate patches and make sure we have requested a valid set.
  const std::vector<CandidatePairs*>& candidatePairs = this->RecordToDisplay.PotentialPairSets;

  unsigned char centerPixelColor[3];
  HelpersQt::QColorToUCharColor(this->Colors.CenterPixel, centerPixelColor);

  for(unsigned int candidateId = 0; candidateId < candidatePairs.size(); ++candidateId)
    {
    unsigned char borderColor[3];
    if(candidateId == this->RecordDisplayState.ForwardLookId)
      {
      HelpersQt::QColorToUCharColor(this->Colors.SelectedForwardLookPatch, borderColor);
      }
    else
      {
      HelpersQt::QColorToUCharColor(this->Colors.ForwardLookPatch, borderColor);
      }

    const Patch& currentPatch = candidatePairs[candidateId]->GetTargetPatch();
    //std::cout << "Outlining " << currentPatch.Region << std::endl;
    //DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);

    ITKVTKHelpers::BlankAndOutlineRegion(this->ForwardLookPatchLayer.ImageData, currentPatch.GetRegion(), borderColor);

    ITKVTKHelpers::SetRegionCenterPixel(this->ForwardLookPatchLayer.ImageData, currentPatch.GetRegion(), centerPixelColor);
    }

  this->Renderer->GetRenderWindow()->Render();
  //LeaveFunction("HighlightForwardLookPatches()");
}
