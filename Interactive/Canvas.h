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

#ifndef Canvas_H
#define Canvas_H

// Custom
#include "Layer.h"

// VTK
class vtkRenderer;

class VTKCanvas
{
public:
  VTKCanvas(vtkRenderer* const renderer);

  // Source patch outline display
  Layer UsedSourcePatchLayer;

  // Target patch outline display
  Layer UsedTargetPatchLayer;

  // Outline display of all forward look patches
  Layer AllForwardLookOutlinesLayer;

  // Outline display of all source patches
  Layer AllSourcePatchOutlinesLayer;

  // Image display
  Layer ImageLayer;

  // Mask image display
  Layer MaskLayer;

private:
  vtkRenderer* Renderer;
};

#endif
