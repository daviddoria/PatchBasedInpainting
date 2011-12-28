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
#include <vtkSeedWidget.h>
#include <vtkPointHandleRepresentation2D.h>

class vtkActor;
class vtkBorderWidget;
class vtkImageData;
class vtkImageSlice;
class vtkImageSliceMapper;
class vtkPolyData;
class vtkPolyDataMapper;

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QThread>

// Custom
#include "ColorPalette.h"
#include "DebugOutputs.h"
#include "DisplayStyle.h"
#include "ImageInput.h"
#include "InpaintingComputationObject.h"
#include "InpaintingIterationRecord.h"
#include "Layer.h"
#include "PatchBasedInpainting.h"
#include "Types.h"
#include "ModelView/TableModelForwardLook.h"
#include "ModelView/TableModelImageInput.h"
#include "ModelView/TableModelTopPatches.h"

class ListModelDisplay;
class ListModelSave;

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

  // Change the camera position to produce the effect of flipping the image.
  void SetCameraPosition();

  void RefreshQt();
  void RefreshVTK();
  void Refresh();

  void AddImageInput(const ImageInput&);

public slots:

  void slot_ChangeFileName(QModelIndex);

  void on_chkDisplayUserPatch_clicked();

  void on_cmbCompareImage_activated(int value);
  void on_cmbSortBy_activated(int value);
  void on_cmbPriority_activated(int value);

  void on_sldDepthColorLambda_valueChanged();

  void on_chkCompareFull_clicked();
  void on_chkCompareColor_clicked();
  void on_chkCompareDepth_clicked();
  void on_chkCompareMembership_clicked();
  void on_chkCompareHistogramIntersection_clicked();

  void on_radDisplayColorImage_clicked();
  void on_radDisplayMagnitudeImage_clicked();
  void on_radDisplayChannel_clicked();
  void on_spinChannelToDisplay_valueChanged(int unused);

//   void on_btnChooseBlurredImage_clicked();
//   void on_btnChooseMembershipImage_clicked();

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

  // Defined in FormGUIElements.cxx
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
  void on_txtGoToIteration_textEdited ( const QString & text );
  void on_txtNumberOfTopPatchesToDisplay_textEdited ( const QString & text );

protected:

  void SetupCamera();
  void SetupScenes();
  
  void ConnectForwardLookModelToView();
  void ConnectTopPatchesModelToView();
  void SetupForwardLookingTable();
  void SetupTopPatchesTable();
  
  void SetProgressBarToMarquee();
  void SetupValidators();
  void SetupLayers();
  void SetupUserPatch();
  void SetupToolbar();
  void SetupComputationThread();
  
  void SetupImageModels();
  
  void SetDefaultValues();

  void showEvent ( QShowEvent * event );

  void Reset();

  std::vector<float> CameraLeftToRightVector;
  std::vector<float> CameraBottomToTopVector;

  void ChangeDisplayedTopPatch();
  void ChangeDisplayedForwardLookPatch();

  // These functions display the iteration indicated by the member 'IterationToDisplay'
  //void DisplayBoundary();
  //void DisplayBoundaryNormals();
  void DisplayMask();
  //void DisplayConfidence();
  //void DisplayConfidenceMap();
  //void DisplayPriority();
  //void DisplayData();
  void DisplayImage();
  void DisplayUserPatch();

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
  Layer UserPatchLayer;

  // Source patch outline display
  Layer UsedSourcePatchLayer;

  // Target patch outline display
  Layer UsedTargetPatchLayer;

  // Outline display of all forward look patches
  Layer AllForwardLookOutlinesLayer;

  // Outline display of all source patches
  Layer AllSourcePatchOutlinesLayer;

  // Image display
  Layer ImageLayer;

  // Boundary image display
  //Layer BoundaryLayer;

  // Mask image display
  Layer MaskLayer;

  // The image that the user loads
  FloatVectorImageType::Pointer UserImage;

  // The mask that the user loads
  Mask::Pointer UserMaskImage;

  // The class that does all the work.
  PatchBasedInpainting* Inpainting;

  // Perform the long inpainting operation in this thread so that the UI remains active.
  InpaintingComputationObject* InpaintingComputation;
  QThread* ComputationThread;

  // A flag that determines if debugging images should be output to files.
  bool DebugImages;

  // A flag that determines if debugging messages should be output.
  bool DebugMessages;

  // If IterationToDisplay == 0, then we are just displaying the initial images.
  unsigned int IterationToDisplay;
  unsigned int ForwardLookToDisplayId;
  unsigned int SourcePatchToDisplayId;
  Patch SourcePatchToDisplay;
  Patch TargetPatchToDisplay;
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

  // Display outlines of where the source patch came from and the target patch to which it was copied
  void HighlightUsedPatches();

  // Display outlines of the forward look target patches
  void HighlightForwardLookPatches();

  // Display outlines of the source patches
  void HighlightSourcePatches();

  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  QGraphicsScene* ResultPatchScene;
  QGraphicsScene* UserPatchScene;

  void OutputPairs(const std::vector<PatchPair>& patchPairs, const std::string& filename);

  // This stores the state of the completion at every step. The index represents the image AFTER the index'th step.
  // That is, the image at index 0 is the image after 0 iterations (the original image). At index 1 is the image after the first target region has been filled, etc.
  std::vector<InpaintingIterationRecord> IterationRecords;

  // Colors
  ColorPalette Colors;


  TableModelForwardLook* ForwardLookModel;
  TableModelTopPatches* TopPatchesModel;

  // The size to display the patches.
  unsigned int PatchDisplaySize;

  // The position of the freely movable patch.
  itk::ImageRegion<2> UserPatchRegion;

  // A Validator to make sure only positive integers can be typed into the text boxes.
  QIntValidator* IntValidator;

  // The radius of the patches.
  unsigned int PatchRadius;
  unsigned int NumberOfTopPatchesToSave;
  unsigned int NumberOfForwardLook;
  unsigned int GoToIteration;
  unsigned int NumberOfTopPatchesToDisplay;

  DisplayStyle ImageDisplayStyle;

  void UserPatchMoved();
  void ComputeUserPatchRegion();

  void SetPriorityFromGUI();
  void SetCompareImageFromGUI();
  void SetComparisonFunctionsFromGUI();
  void SetSortFunctionFromGUI();
  void SetDepthColorLambdaFromGUI();
  void SetParametersFromGUI();

  void SetupConnections();

  QVector<ImageInput> ImageInputs;

  ListModelSave* ModelSave;
  ListModelDisplay* ModelDisplay;
  TableModelImageInput* ModelImages;

  void UpdateAllImageInputModels();

  void OpenInputImages();

  NamedITKImageCollection InputImages;

  void SetupSaveModel();
};

#endif // PatchBasedInpaintingGUI_H
