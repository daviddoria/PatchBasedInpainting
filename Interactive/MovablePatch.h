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

#ifndef MovablePatch_H
#define MovablePatch_H

// Custom
#include "Layer.h"
class InteractorStyleImageWithDrag;

// ITK
#include "itkImageRegion.h"

// Qt
#include <QColor>
#include <QGraphicsScene>
class QGraphicsView;

// VTK
class vtkRenderer;
#include <vtkImageSlice.h>

class MovablePatch : public QObject
{
Q_OBJECT

signals:
  void signal_PatchMoved();

public:
  /** This constructor is provided so that we can store a MovablePatch as a member,
   * but not initialize it until after the renderers/interactors are setup and configured.*/
  MovablePatch();

  /** This constructor is provided if everything is known when we create the object.*/
  //MovablePatch(const unsigned int radius, vtkRenderer* const renderer,
  MovablePatch(const unsigned int radius, InteractorStyleImageWithDrag* const interactorStyle,
               QGraphicsView* const view = NULL, const QColor color = QColor());

  void SetVisibility(const bool);
  bool GetVisibility();

  /** The ITK region describing the position of the patch. */
  itk::ImageRegion<2> GetRegion();

  /** Set a GraphicsView in which to display the patch. */
  void SetGraphicsView(QGraphicsView* const view);

  void Display();

private:
  Layer PatchLayer;
  //Layer* PatchLayer;

  void PatchMoved();

  unsigned int Radius;
  InteractorStyleImageWithDrag* InteractorStyle;
  QGraphicsView* View;
  QColor Color;

  QSharedPointer<QGraphicsScene> PatchScene;

  QImage PatchImage;
};

#endif
