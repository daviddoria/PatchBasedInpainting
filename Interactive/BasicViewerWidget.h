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

#ifndef PatchBasedInpaintingViewerWidget_H
#define PatchBasedInpaintingViewerWidget_H

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
#include "Node.h"
#include "Interactive/Layer.h"

class InteractorStyleImageWithDrag;

class PatchBasedInpaintingViewerWidgetParent : public QMainWindow, public Ui::PatchBasedInpaintingViewerWidget
{
Q_OBJECT

public slots:

  virtual void slot_UpdateImage() = 0;
  virtual void slot_UpdateSource(const itk::ImageRegion<2>& region) = 0;
  virtual void slot_UpdateTarget(const itk::ImageRegion<2>& region) = 0;

};

template <typename TImage>
class PatchBasedInpaintingViewerWidget : public PatchBasedInpaintingViewerWidgetParent
{
private:
  /** The image that will be displayed, and the from which the patches will be extracted before being displayed. */
  TImage* Image;

  /** This variable is used to track whether or not the image size changed between this refresh and the last refresh.
   * Typically it is simply used to determine if ResetCamera should be called before rendering. We typically do not
   * want to call ResetCamera if only the image content has been changed, but we do want to call it if the image
   * size has changed (typically this only when the image is changed, or setup for the first time). */
  int ImageDimension[3];

  /** A wrapper that creates and holds the image, the mapper, and the actor. */
  Layer ImageLayer;

public:
  // Constructor
  PatchBasedInpaintingViewerWidget(TImage* const image);

  void slot_UpdateImage();
  void slot_UpdateSource(const itk::ImageRegion<2>& region);
  void slot_UpdateTarget(const itk::ImageRegion<2>& region);

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

  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  QGraphicsScene* ResultPatchScene;

  // The color to use as the background of the QGraphicsScenes
  QColor SceneBackground;

  // Connect all signals and slots.
  void SetupConnections();

  ImageCamera* Camera;
};

#include "BasicViewerWidget.hpp"

#endif // PatchBasedInpaintingViewerWidget_H
