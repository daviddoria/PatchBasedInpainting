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

#ifndef PatchBasedInpaintingGUI_H
#define PatchBasedInpaintingGUI_H

#include "ui_PatchBasedInpainting.h"

// VTK
#include <vtkSmartPointer.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QThread>

// Custom
#include "InpaintingIterationRecordViewer.h"

#include "DebugOutputs.h"
#include "DisplayState.h"
#include "ImageCamera.h"
#include "ImageInput.h"
#include "InpaintingComputationObject.h"
#include "InpaintingIterationRecord.h"
#include "MovablePatch.h"
#include "PatchBasedInpainting.h"
#include "InpaintingDisplaySettings.h"
#include "Types.h"

#include "ModelView/TableModelForwardLook.h"
#include "ModelView/TableModelImageInput.h"
#include "ModelView/TableModelTopPatches.h"
#include "ModelView/ListModelDisplay.h"
#include "ModelView/ListModelSave.h"

class InteractorStyleImageWithDrag;

class PatchBasedInpaintingGUI : public QMainWindow, public Ui::PatchBasedInpaintingGUI, public DebugOutputs
{
  Q_OBJECT
public:

  // Constructor/Destructor
  void DefaultConstructor();
  PatchBasedInpaintingGUI();
  PatchBasedInpaintingGUI(const std::string& imageFileName, const std::string& maskFileName, const bool debugEnterLeave);
  ~PatchBasedInpaintingGUI() {};

  void RefreshQt();
  void RefreshVTK();
  void Refresh();

  void AddImageInput(const ImageInput&);

public slots:

  void slot_ChangeFileName(QModelIndex);

  void on_chkDisplayUserPatch_clicked();

  void on_cmbCompareImage_activated(int value);
  void on_cmbPriority_activated(int value);

  void on_radDisplayColorImage_clicked();
  void on_radDisplayMagnitudeImage_clicked();
  void on_radDisplayChannel_clicked();
  void on_spinChannelToDisplay_valueChanged(int unused);

  void on_btnGoToIteration_clicked();

  void on_btnDisplayPreviousStep_clicked();
  void on_btnDisplayNextStep_clicked();

  void on_actionOpen_activated();
  void on_actionSaveResult_activated();

  void on_chkHighlightUsedPatches_clicked();

  void on_chkDebugImages_clicked();
  void on_chkDebugMessages_clicked();

  void on_chkLive_clicked();

  void slot_ForwardLookTableView_changed(const QModelIndex& currentIndex, const QModelIndex& previousIndex);
  void slot_TopPatchesTableView_changed(const QModelIndex& currentIndex, const QModelIndex& previousIndex);

  void on_chkDisplayForwardLookPatchLocations_clicked();
  void on_chkDisplaySourcePatchLocations_clicked();

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

  void on_txtPatchRadius_textEdited ( const QString & text );
  void on_txtNumberOfTopPatchesToSave_textEdited ( const QString & text );
  void on_txtNumberOfForwardLook_textEdited ( const QString & text );
  void on_txtNumberOfTopPatchesToDisplay_textEdited ( const QString & text );

private:

  void SetupScenes();

  void ConnectForwardLookModelToView();
  void ConnectTopPatchesModelToView();
  void SetupForwardLookingTable();
  void SetupTopPatchesTable();

  void SetProgressBarToMarquee();
  void SetupValidators();
  void SetupToolbar();
  void SetupComputationThread();

  void SetupImageModels();

  void showEvent ( QShowEvent * event );

  void Reset();

  void ChangeDisplayedTopPatch();
  void ChangeDisplayedForwardLookPatch();

  // These functions display the iteration indicated by the member 'IterationToDisplay'
  void DisplayMask();
  void DisplayImage();

  void OpenImage(const std::string& filename);
  void OpenMask(const std::string& filename, const bool inverted);

  // Initialize everything.
  void Initialize();

  // Copy the initial state into the first record
  void CreateInitialRecord();

  // Save everything at the end of an iteration.
  void IterationComplete(const PatchPair& patchPair);

  // This function is called when the "Previous" or "Next" buttons are pressed, and at the end of IterationComplete().
  void ChangeDisplayedIteration();

  // The interactor to allow us to zoom and pan the image while still moving images with Pickable=true
  vtkSmartPointer<InteractorStyleImageWithDrag> InteractorStyle;

  // Track if the image has been flipped
  //bool Flipped;

  // The only renderer
  vtkSmartPointer<vtkRenderer> Renderer;

  // A patch that the user can move around freely.
  std::shared_ptr<MovablePatch> UserPatch;

  // This is a pointer because it must be initialized with a Renderer
  std::shared_ptr<InpaintingIterationRecordViewer> RecordViewer;

  // The image that the user loads
  FloatVectorImageType::Pointer UserImage;

  // The mask that the user loads
  Mask::Pointer UserMaskImage;

  // The class that does all the work.
  PatchBasedInpainting<FloatVectorImageType>* Inpainting;

  // Perform the long inpainting operation in this thread so that the UI remains active.
  InpaintingComputationObject* InpaintingComputation;
  QThread* ComputationThread;

  InpaintingIterationRecord* RecordToDisplay;

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

  // This stores the state of the completion at every step. The index represents the image AFTER the index'th step.
  // That is, the image at index 0 is the image after 0 iterations (the original image). At index 1 is the image after the first target region has been filled, etc.
  std::vector<InpaintingIterationRecord> IterationRecords;

  // The color to use as the background of the QGraphicsScenes
  QColor SceneBackground;

  // Models and views
  QSharedPointer<TableModelForwardLook> ForwardLookModel;
  QSharedPointer<TableModelTopPatches> TopPatchesModel;

  QSharedPointer<ListModelSave> ModelSave;
  QSharedPointer<ListModelDisplay> ModelDisplay;
  QSharedPointer<TableModelImageInput> ModelImages;

  void UpdateAllImageInputModels();

  void SetupSaveModel();

  // A Validator to make sure only positive integers can be typed into the text boxes.
  // Since we provide a parent, this does not need to be a smart pointer.
  QIntValidator* IntValidator;

  // Make sure only valid iterations can be typed.
  // Since we provide a parent, this does not need to be a smart pointer.
  QIntValidator* IterationValidator;

  InpaintingDisplaySettings Settings;

  DisplayState RecordDisplayState;

  // This function calls several functions that set parameters from GUI values.
  void SetParametersFromGUI();

  // These functions set specific parameters from GUI values.
  void SetPriorityFromGUI();
  void SetCompareImageFromGUI();
  void SetComparisonFunctionsFromGUI();
  void SetSortFunctionFromGUI();

  // Connect all signals and slots.
  void SetupConnections();

  QVector<ImageInput> ImageInputs;

  void OpenInputImages();

  NamedITKImageCollection InputImages;

  ImageCamera* Camera;
};

#endif // PatchBasedInpaintingGUI_H
