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

/* This widget configures the options of a PatchBasedInpainting object
 * and visualizes the output at each iteration. The PatchBasedInpainting
 * is not created until the Initialize button is clicked.
*/

#ifndef InpaintingIterationRecordViewer_H
#define InpaintingIterationRecordViewer_H

// Custom
#include "ColorPalette.h"
#include "DisplayStyle.h"
#include "DisplayState.h"
#include "InpaintingIterationRecord.h"
#include "Layer.h"
#include "Mask.h"
#include "PatchPair.h"
#include "InpaintingDisplaySettings.h"

// STL
#include <vector>

// VTK
class vtkRenderer;

// This class is responsible for 
class InpaintingIterationRecordViewer
{
public:
  InpaintingIterationRecordViewer(vtkRenderer* const renderer);

  DisplayStyle const& GetImageDisplayStyle() const;
  DisplayStyle& GetImageDisplayStyle();

  ColorPalette const& GetColorPalette() const;

  void SetDisplayStyle(const DisplayStyle& style);
  void SetDisplayState(const DisplayState& displayStyle);
  void SetSettings(const InpaintingDisplaySettings& settings);

  void DisplayRecord(const InpaintingIterationRecord& record);

private:

  InpaintingIterationRecord RecordToDisplay;

  // Display outlines of where the source patch came from and the target patch to which it was copied
  void HighlightUsedPatches();

  // Display outlines of the forward look target patches
  void HighlightForwardLookPatches();

  // Display outlines of the source patches
  void HighlightSourcePatches();

  void HighlightUsedPatchPair(const PatchPair& patchPair);
  void HighlightForwardLookPatches(const std::vector<Patch>& patches);
  void HighlightSourcePatches(const std::vector<Patch>& patches);

  // Source patch outline display
  Layer UsedPatchPairLayer;

  // Outline display of all forward look patches
  Layer ForwardLookPatchLayer;

  // Outline display of all source patches
  Layer SourcePatchLayer;

  // Image display
  Layer ImageLayer;

  // Mask image display
  Layer MaskLayer;

  void DisplayImage(const FloatVectorImageType* const image);
  void DisplayMask(const Mask* const mask);

  void ITKVectorImageToVTKImage(const FloatVectorImageType* const image, vtkImageData* outputImage);

  void HighlightPatches(const std::vector<Patch>& patches, const QColor& color, vtkImageData* outputImage);

  vtkRenderer* Renderer;

  DisplayStyle ImageDisplayStyle;

  DisplayState RecordDisplayState;

  InpaintingDisplaySettings DisplaySettings;

  ColorPalette Colors;
};

#endif
