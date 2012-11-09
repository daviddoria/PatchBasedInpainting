/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef ManualPatchSelectionDialog_H
#define ManualPatchSelectionDialog_H

#include "ManualPatchSelectionDialogParent.h"

// VTK
#include <vtkSmartPointer.h>

// ITK
#include "itkImage.h"

// Submodules
#include <Mask/Mask.h>
#include <ITKVTKCamera/ITKVTKCamera.h>

// Custom
#include "Node.h"
#include "Interactive/Layer.h"
#include "Interactive/MovablePatch.h"

class InteractorStyleImageWithDrag;

/** When all else fails (acceptance tests failed and TopPatchesDialog doesn't contain an acceptable patch
  * for the user to select), we can use this class to allow the user to manually position a box on the image
  * indicating the patch they would like to use to fill the target patch.
  */
template <typename TImage>
class ManualPatchSelectionDialog : public ManualPatchSelectionDialogParent
{

public:
  /** Constructor */
  ManualPatchSelectionDialog(const TImage* const image, const Mask* const mask,
                             const itk::ImageRegion<2>& targetRegion);

  /** Destructor */
  ~ManualPatchSelectionDialog();

  /** We need the target region as well while updating the source region because we may want to mask
   * the source patch with the target patch's mask. */
  void slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

  /** Update the target patch image. */
  void slot_UpdateTarget(const itk::ImageRegion<2>& region);

  /** Update the result patch image. */
  void slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

  /** When the user clicks Accept, store the selected node for retrieval. */
  void on_btnAccept_clicked();

  /** When the patch is moved by the user, we need to update the zoomed-in displays of the patches. */
  void slot_PatchMoved();

  /** The caller will use this function to get the selected node. */
  Node GetSelectedNode();
  
  /** Get the number of times this class has been used. */
  void GetNumberOfUses();

private:

  /** This variable tracks how many times the patch was selected by choosing it from the TopPatchesDialog. */
  unsigned int NumberOfUses = 0;

  /** Do some things after the widget is displayed. */
  void showEvent(QShowEvent* event);
  
  /** Gracefully quit if the user closes the dialog. */
  void closeEvent(QCloseEvent*);

  /** The image that will be displayed, and from which the patches will be extracted before being displayed. */
  const TImage* Image;

  /** The mask that will be used to mask the patches that are displayed. */
  const Mask* MaskImage;
  
  /** Setup the QGraphicScene objects. */
  void SetupScenes();

  /** The interactor to allow us to zoom and pan the image while still moving images with Pickable=true */
  vtkSmartPointer<InteractorStyleImageWithDrag> InteractorStyle;

  /** The renderer. */
  vtkSmartPointer<vtkRenderer> Renderer;

  /** The QGraphicScene objects for the source, target, and result patches. */
  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  QGraphicsScene* ResultPatchScene;

  /** The color to use as the background of the QGraphicsScenes */
  QColor SceneBackground;

  /** Connect all signals and slots. */
  void SetupConnections();

  /** This variable is used to track whether or not the image size changed between this refresh and the last refresh.
   * Typically it is simply used to determine if ResetCamera should be called before rendering. We typically do not
   * want to call ResetCamera if only the image content has been changed, but we do want to call it if the image
   * size has changed (typically this only when the image is changed, or setup for the first time). */
  int ImageDimension[3];

  /** A wrapper that creates and holds the image, the mapper, and the actor for the main image. */
  Layer ImageLayer;

  /** The patch that will be moved around by the user. This MUST be a pointer,
   *  because the constructor registers 'this' as a VTK observer, and if it is done from a temporary
   *  (i.e. PatchSelector = MovablePatch(...)), 'this' changes when the assignment operator copies the
   *  resulting object into PatchSelector. */
  MovablePatch<InteractorStyleImageWithDrag>* SourcePatchSelector;

  /** This is used to display the target patch on top of the image. Though it is a MovablePatch, we disable
    * the movability (it is fixed in place). */
  MovablePatch<InteractorStyleImageWithDrag>* TargetPatchDisplayer;

  /** The patch region that we are trying to pick a match for. */
  itk::ImageRegion<2> TargetRegion;

  /** The node that was selected by the user. We have to store it here because it has to be retrieved by the caller. */
  Node SelectedNode;

  /** A helper object that sets up the viewing orientation of the image. */
  ITKVTKCamera* ItkVtkCamera;
};

#include "ManualPatchSelectionDialog.hpp"

#endif // ManualPatchSelectionDialog_H
