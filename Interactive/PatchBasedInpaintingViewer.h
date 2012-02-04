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

#ifndef PatchBasedInpaintingViewer_H
#define PatchBasedInpaintingViewer_H

#include "ui_PatchBasedInpaintingViewer.h"

// VTK
#include <vtkSmartPointer.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QThread>

// Custom
#include "DisplayState.h"
#include "ImageCamera.h"
#include "ImageInput.h"
#include "InpaintingComputationObject.h"
#include "InpaintingIterationRecord.h"
#include "MovablePatch.h"
#include "PatchBasedInpainting.h"
#include "InpaintingDisplaySettings.h"
#include "Types.h"

class InteractorStyleImageWithDrag;

class PatchBasedInpaintingGUI : public QMainWindow, public Ui::PatchBasedInpaintingViewer
{
  Q_OBJECT
public:

  // Constructor/Destructor
  void DefaultConstructor();
  PatchBasedInpaintingViewer();
  PatchBasedInpaintingViewer(const std::string& imageFileName, const std::string& maskFileName, const bool debugEnterLeave);
  ~PatchBasedInpaintingViewer() {};

  void RefreshQt();
  void RefreshVTK();
  void Refresh();

public slots:

  void slot_ChangeFileName(QModelIndex);

  void on_chkDisplayUserPatch_clicked();

  void on_cmbCompareImage_activated(int value);
  void on_cmbPriority_activated(int value);

  void on_radDisplayColorImage_clicked();
  void on_radDisplayMagnitudeImage_clicked();
  void on_radDisplayChannel_clicked();
  void on_spinChannelToDisplay_valueChanged(int unused);

  void on_actionOpen_activated();
  void on_actionSaveResult_activated();

  void on_btnInitialize_clicked();
  void on_btnInpaint_clicked();
  void on_btnStep_clicked();
  void on_btnStop_clicked();
  void on_btnReset_clicked();

  void on_actionHelp_activated();
  void on_actionQuit_activated();

  void on_actionFlipImageVertically_activated();
  void on_actionFlipImageHorizontally_activated();

  void slot_StartProgress();
  void slot_StopProgress();

  void slot_IterationComplete(const PatchPair&);

  void slot_ChangeDisplayedImages(QModelIndex);

private:

  void SetupScenes();

  void SetProgressBarToMarquee();
  void SetupValidators();
  void SetupToolbar();
  void SetupComputationThread();

  void SetupImageModels();

  void showEvent ( QShowEvent * event );

  void Reset();

  // These functions display the iteration indicated by the member 'IterationToDisplay'
  void DisplayMask();
  void DisplayImage();

  void OpenImage(const std::string& filename);
  void OpenMask(const std::string& filename, const bool inverted);

  // Initialize everything.
  void Initialize();

  // Save everything at the end of an iteration.
  void IterationComplete(const PatchPair& patchPair);

  // The interactor to allow us to zoom and pan the image while still moving images with Pickable=true
  vtkSmartPointer<InteractorStyleImageWithDrag> InteractorStyle;

  // The only renderer
  vtkSmartPointer<vtkRenderer> Renderer;

  // The image that the user loads
  FloatVectorImageType::Pointer UserImage;

  // The mask that the user loads
  Mask::Pointer UserMaskImage;

  // The class that does all the work.
  PatchBasedInpainting<FloatVectorImageType>* Inpainting;

  // Display zoomed in versions of the patches used at the current iteration
  void DisplayUsedPatches();
  void DisplaySourcePatch();
  void DisplayTargetPatch();
  void DisplayResultPatch();

  // Make sure internal variables and the state of the GUI elements matches.
  void InitializeGUIElements();

  // Display the text information (scores, etc) of the patches used at the current information
  void DisplayUsedPatchInformation();

  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  QGraphicsScene* ResultPatchScene;

  // The color to use as the background of the QGraphicsScenes
  QColor SceneBackground;

  // A Validator to make sure only positive integers can be typed into the text boxes.
  // Since we provide a parent, this does not need to be a smart pointer.
  QIntValidator* IntValidator;

  InpaintingDisplaySettings Settings;

  // Connect all signals and slots.
  void SetupConnections();

  ImageCamera* Camera;
};

#endif // PatchBasedInpaintingViewer_H
