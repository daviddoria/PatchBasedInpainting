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

#ifndef BasicViewerWidget_H
#define BasicViewerWidget_H

#include "ui_BasicViewerWidget.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkInteractorStyleImage.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QThread>

// Submodules
#include <ITKVTKCamera/ITKVTKCamera.h>
#include <Mask/Mask.h>

// Custom
#include "Node.h"
#include "Interactive/Layer.h"
#include "Interactive/PatchHighlighter.h"

class InteractorStyleImageWithDrag;

/** Class templates cannot have slots directly, so this class effectively lets BasicViewerWidget
  * have slots.
  */
class BasicViewerWidgetParent : public QMainWindow, public Ui::BasicViewerWidget
{
Q_OBJECT

private:
protected:
  /** Gracefully quit if the user closes the dialog. */
  virtual void closeEvent(QCloseEvent* event) = 0;

public slots:

  /** Update the image that is displayed. */
  virtual void slot_UpdateImage() = 0;

  /** Update the source patch that is outlined.
    * This function needs the targetRegion because this is the region
    * of the Mask that is used to mask the source patch.
    */
  virtual void slot_UpdateSource(const itk::ImageRegion<2>& region,
                                 const itk::ImageRegion<2>& targetregion) = 0;

  virtual void slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion) = 0;

  /** Update the target patch that is outlined. */
  virtual void slot_UpdateTarget(const itk::ImageRegion<2>& region) = 0;

};

template <typename TImage>
class BasicViewerWidget : public BasicViewerWidgetParent
{
private:
  void closeEvent(QCloseEvent*);

  /** The image that will be displayed, and the from which the patches will
    * be extracted before being displayed. */
//  TImage* Image;
  typename TImage::Pointer Image;

  /** The mask that will be used to mask the patches that are displayed. */
//  Mask* MaskImage;
  Mask::Pointer MaskImage;

  /** This variable is used to track whether or not the image size changed between this refresh and the last refresh.
   * Typically it is simply used to determine if ResetCamera should be called before rendering. We typically do not
   * want to call ResetCamera if only the image content has been changed, but we do want to call it if the image
   * size has changed (typically this only when the image is changed, or setup for the first time). */
  int ImageDimension[3];

  /** A wrapper that creates and holds the image, the mapper, and the actor. */
  Layer ImageLayer;

public:
  // Constructor
//  BasicViewerWidget(TImage* const image, Mask* const mask);
  BasicViewerWidget(typename TImage::Pointer image, Mask::Pointer mask);

  void slot_UpdateImage();

  /** Update the source region outline, and display the proposed source patch.
    * We need the target region as well while updating the
    * source region because we may want to mask the source patch with the target patch's mask.
    */
  void slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion,
                         const itk::ImageRegion<2>& targetRegion);

  /** Update the source region outline.*/
  void slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion);

  /** Update the target region outline and display the target patch.*/
  void slot_UpdateTarget(const itk::ImageRegion<2>& region);

  /** Make all of the necessary connection for a visitor to drive this viewer.*/
  template <typename TVisitor>
  void ConnectVisitor(TVisitor* visitor);

  /** Make all of the necessary connection for a widget to drive this viewer.*/
  template <typename TPatchesWidget>
  void ConnectWidget(TPatchesWidget* widget);

private:

  /** Setup the QGraphicsScenes. */
  void SetupScenes();

  /** The interactor to allow us to zoom and pan the image while still moving images with Pickable=true */
//  vtkSmartPointer<InteractorStyleImageWithDrag> InteractorStyle;
  vtkSmartPointer<vtkInteractorStyleImage> InteractorStyle;

  /** The renderer */
  vtkSmartPointer<vtkRenderer> Renderer;

  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  QGraphicsScene* MaskedSourcePatchScene;
  QGraphicsScene* MaskedTargetPatchScene;

  /** The color to use as the background of the QGraphicsScenes */
  QColor SceneBackground;

  /** Connect all signals and slots. */
  void SetupConnections();

  /** An object that sets up the viewing orientation of the image. */
  ITKVTKCamera* ItkVtkCamera;

  /** The object that indicates where the source patch is located. */
  PatchHighlighter* SourceHighlighter;

  /** The object that indicates where the target patch is located. */
  PatchHighlighter* TargetHighlighter;
};

/** BasicViewerWidgetWrapper is a class template and therefore cannot have slots directly.
  * We need this parent that is a QObject to provide the slots. These slots should be identical to
  * those in BasicViewerWidgetParent as we will be forwarding them.
  */
class BasicViewerWidgetWrapperParent : public QObject
{
Q_OBJECT

public slots:
  void slot_UpdateImage()
  {
    signal_UpdateImage();
  }

  // We need the target region as well while updating the source region because we may want to mask the source patch with the target patch's mask.
  void slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
  {
    signal_UpdateSource(sourceRegion, targetRegion);
  }

  void slot_UpdateTarget(const itk::ImageRegion<2>& region)
  {
    signal_UpdateTarget(region);
  }

  void slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
  {
    signal_UpdateResult(sourceRegion, targetRegion);
  }

signals:
  // We mirror the slots with the same signals. We will use these to do the forwarding.

  void signal_UpdateImage();

  // We need the target region as well while updating the source region because we may want to mask the source patch with the target patch's mask.
  void signal_UpdateSource(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

  void signal_UpdateTarget(const itk::ImageRegion<2>& region);
  void signal_UpdateResult(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

};

/** QWidget subclasses cannot be created from threads other than the main GUI thread (and also cannot be created there and moved to the GUI thread).
  * To allow us to do this, we need this wrapper, which we can create, move to the GUI thread, and then forward signals that it receives to the internal BasicViewerWidget.
  * Additionally, this class template cannot have slots directly, so we need a parent that is a QObject.
  */
template <typename TImage>
class BasicViewerWidgetWrapper : public BasicViewerWidgetWrapperParent
{

public:
  BasicViewerWidgetWrapper(TImage* const image, Mask* const mask)
  {
    this->InternalBasicViewerWidget = new BasicViewerWidget<TImage>(image, mask);

    QObject::connect(this, SIGNAL(signal_UpdateImage()), InternalBasicViewerWidget, SLOT(slot_UpdateImage()),
                     Qt::BlockingQueuedConnection);

    QObject::connect(this, SIGNAL(signal_UpdateSource(const itk::ImageRegion<2>&, const itk::ImageRegion<2>&)),
                     InternalBasicViewerWidget, SLOT(slot_UpdateSource(const itk::ImageRegion<2>&, const itk::ImageRegion<2>&)),
                     Qt::BlockingQueuedConnection);

    QObject::connect(this, SIGNAL(signal_UpdateTarget(const itk::ImageRegion<2>&)),
                     InternalBasicViewerWidget, SLOT(slot_UpdateTarget(const itk::ImageRegion<2>&)),
                     Qt::BlockingQueuedConnection);

    QObject::connect(this, SIGNAL(signal_UpdateResult(const itk::ImageRegion<2>&, const itk::ImageRegion<2>&)),
                     InternalBasicViewerWidget, SLOT(slot_UpdateResult(const itk::ImageRegion<2>&, const itk::ImageRegion<2>&)),
                     Qt::BlockingQueuedConnection);
  }

  void show()
  {
    InternalBasicViewerWidget->show();
  }

private:
  BasicViewerWidget<TImage>* InternalBasicViewerWidget;

};

#include "BasicViewerWidget.hpp"

#endif // BasicViewerWidget_H
