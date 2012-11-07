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

#ifndef PriorityViewerWidget_H
#define PriorityViewerWidget_H

#include "ui_PriorityViewerWidget.h"

// VTK
#include <vtkSmartPointer.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QThread>

// Submodules
#include <ITKVTKCamera/ITKVTKCamera.h>
#include <Mask/Mask.h>

// Custom
#include "ImageCamera.h"
#include "Node.h"
#include "Interactive/Layer.h"
#include "Priority/Priority.h"

class InteractorStyleImageWithDrag;

class PriorityViewerWidgetParent : public QMainWindow, public Ui::PriorityViewerWidget
{
Q_OBJECT

public slots:

  /** Update the image with the current hole boundary and priority values. */
  virtual void slot_UpdateImage() = 0;

};

template <typename TPriority, typename TBoundaryStatusMapType>
class PriorityViewerWidget : public PriorityViewerWidgetParent
// class PriorityViewerWidget : public QMainWindow, public Ui::PriorityViewerWidget
{
private:

  /** The image whose pixels indicate the priority of each pixel. */
  typedef itk::Image<float, 2> PriorityImageType;
  PriorityImageType::Pointer PriorityImage;
  
  /** The function to use to compute the priority of a pixel. */
  TPriority* PriorityFunction;

  /** The size of the image. */
  itk::Size<2> ImageSize;

  /** This variable is simply to determine whether or not to ResetCamera. */
  bool PreviouslyDisplayed;
  
  /** This variable is used to track whether or not the image size changed between this refresh and the last refresh.
   * Typically it is simply used to determine if ResetCamera should be called before rendering. We typically do not
   * want to call ResetCamera if only the image content has been changed, but we do want to call it if the image
   * size has changed (typically this only when the image is changed, or setup for the first time). */
  int ImageDimension[3];

  /** A wrapper that creates and holds the image, the mapper, and the actor. */
  Layer ImageLayer;

  /** A data structure that indicates if each pixel is on the boundary. */
  TBoundaryStatusMapType* BoundaryStatusMap;

public:
  /** Constructor. */
  PriorityViewerWidget(TPriority* const priorityFunction, const itk::Size<2>& imageSize,
                       TBoundaryStatusMapType* const boundaryStatusMap);

  void slot_UpdateImage();

private:

  /** The interactor to allow us to zoom and pan the image while still moving images with Pickable=true. */
  vtkSmartPointer<InteractorStyleImageWithDrag> InteractorStyle;

  /** The renderer. */
  vtkSmartPointer<vtkRenderer> Renderer;

  /** An object that sets up the viewing orientation of the image. */
  ITKVTKCamera* ItkVtkCamera;

};

#include "PriorityViewerWidget.hpp"

#endif // PriorityViewerWidget_H
