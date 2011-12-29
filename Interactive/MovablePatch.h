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

#ifndef MovablePatch_H
#define MovablePatch_H

// Custom
#include "Layer.h"

// ITK
#include "itkImageRegion.h"

// Qt
#include <QGraphicsScene>
class QGraphicsView;

// VTK
class vtkRenderer;

class MovablePatch
{
public:
  MovablePatch(const unsigned int radius, vtkRenderer* const renderer, QGraphicsView* const view, const QColor color = QColor());

  void SetVisibility(const bool);
  bool GetVisibility();

  // The ITK region describing the position of the patch.
  itk::ImageRegion<2> GetRegion();

  void Display();

private:
  Layer PatchLayer;

  void PatchMoved();

  unsigned int Radius;
  vtkRenderer* Renderer;
  QGraphicsView* const View;
  QColor Color;

  QSharedPointer<QGraphicsScene> PatchScene;

  QImage PatchImage;
};

#endif
