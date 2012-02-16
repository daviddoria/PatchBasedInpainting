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

#ifndef PatchBasedInpaintingViewer_H
#define PatchBasedInpaintingViewer_H

#include "ui_PatchBasedInpaintingViewerWidget.h"

// VTK
#include <vtkSmartPointer.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QThread>

// Custom
#include "ImageCamera.h"
#include "ImageProcessing/Mask.h"

class InteractorStyleImageWithDrag;

class PatchBasedInpaintingViewerWidget : public QMainWindow, public Ui::PatchBasedInpaintingViewerWidget
{
  Q_OBJECT
public:

  // Constructor
  PatchBasedInpaintingViewerWidget();

public slots:

private:

  void SetupScenes();

  // The interactor to allow us to zoom and pan the image while still moving images with Pickable=true
  vtkSmartPointer<InteractorStyleImageWithDrag> InteractorStyle;

  // The only renderer
  vtkSmartPointer<vtkRenderer> Renderer;

  // The image that the user loads
  FloatVectorImageType::Pointer UserImage;

  // The mask that the user loads
  Mask::Pointer UserMaskImage;

  // Display zoomed in versions of the patches used at the current iteration
  void DisplaySourcePatch();
  void DisplayTargetPatch();
  void DisplayResultPatch();

  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  QGraphicsScene* ResultPatchScene;

  // The color to use as the background of the QGraphicsScenes
  QColor SceneBackground;

  // Connect all signals and slots.
  void SetupConnections();

  ImageCamera* Camera;
};

#endif // PatchBasedInpaintingViewerWidget_H
