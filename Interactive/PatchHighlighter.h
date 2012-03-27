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

/*
*/

#ifndef PatchHighlighter_H
#define PatchHighlighter_H

// Custom
#include "Layer.h"

// ITK
#include "itkImageRegion.h"

// Qt
#include <QColor>
#include <QObject>

// VTK
class vtkRenderer;
#include <vtkImageSlice.h>

class PatchHighlighter
{

public:
  /** This constructor is provided so that we can store a PatchHighlighter as a member,
   * but not initialize it until after the renderers/interactors are setup and configured.*/
  PatchHighlighter();

  /** This constructor is provided if everything is known when we create the object.*/
  //MovablePatch(const unsigned int radius, vtkRenderer* const renderer,
  PatchHighlighter(const unsigned int radius, vtkRenderer* const renderer,
                   const QColor color = QColor());

  void SetVisibility(const bool);
  bool GetVisibility();

  /** Get the ITK region describing the position of the patch. */
  itk::ImageRegion<2> GetRegion();

  /** Set the position of the patch. */
  void SetRegion(const itk::ImageRegion<2>&);

private:
  Layer PatchLayer;

  unsigned int Radius;

  QColor Color;

  vtkRenderer* Renderer;

};

#endif
