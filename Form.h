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

#ifndef FORM_H
#define FORM_H

#include "ui_CriminisiInpainting.h"

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

// Custom
#include "CriminisiInpainting.h"
#include "InpaintingVisualizationStack.h"
#include "Layer.h"
#include "ProgressThread.h"
#include "Types.h"
#include "VectorLayer.h"

class InteractorStyleImageNoLevel;

class Form : public QMainWindow, public Ui::CriminisiInpaintingMainWindow
{
  Q_OBJECT
public:

  // Constructor/Destructor
  void DefaultConstructor();
  Form();
  Form(const std::string& imageFileName, const std::string& maskFileName);
  ~Form() {};
  
  // These function deal with flipping the image
  void SetCameraPosition(double leftToRight[3], double bottomToTop[3]);
  void SetCameraPosition1();
  void SetCameraPosition2();
  
  void Refresh();
  
public slots:
  
  void on_radCompareOriginal_clicked();
  void on_radCompareBlurred_clicked();
  void on_radCompareCIELAB_clicked();
  
  void on_btnGoToIteration_clicked();
  
  void on_btnDisplayPreviousStep_clicked();
  void on_btnDisplayNextStep_clicked();
  
  void on_actionOpen_activated();
  void on_actionSaveResult_activated();
  
  void on_chkHighlightUsedPatches_clicked();
  
  void on_chkDebugImages_clicked();
  void on_chkDebugMessages_clicked();
  
  void on_chkLive_clicked();

  void on_forwardLookingTableWidget_currentCellChanged(int row, int col, int prevRow, int prevCol);
  void on_topPatchesTableWidget_currentCellChanged(int row, int col, int prevRow, int prevCol);
  
  // Defined in FormGUIElements.cxx
  void on_chkImage_clicked();
  void on_chkMask_clicked();
  void on_chkPriority_clicked();
  void on_chkConfidence_clicked();
  void on_chkConfidenceMap_clicked();
  void on_chkBoundary_clicked();
  void on_chkIsophotes_clicked();
  void on_chkData_clicked();
  void on_chkBoundaryNormals_clicked();
  void on_chkPotentialPatches_clicked();
  void on_chkDisplayForwardLookPatchLocations_clicked();
  void on_chkDisplaySourcePatchLocations_clicked();
  
  void SetCheckboxVisibility(const bool visible);
  
  void on_btnInpaint_clicked();
  void on_btnStep_clicked();
  void on_btnInitialize_clicked();
  void on_btnStop_clicked();
  void on_btnReset_clicked();
  
  void on_btnResort_clicked();
  
  void on_actionHelp_activated();
  void on_actionQuit_activated();
  
  void on_actionFlipImage_activated();
  
  void StartProgressSlot();
  void StopProgressSlot();
  
  void RefreshSlot();
  
  void IterationCompleteSlot();
  
  void DisplayIsophotes();
  
protected:
  
  void ChangeDisplayedTopPatch();
  void ChangeDisplayedForwardLookPatch();
  
  // These functions display the iteration indicated by the member 'IterationToDisplay'
  void DisplayBoundary();
  void DisplayBoundaryNormals();
  void DisplayMask();
  void DisplayConfidence();
  void DisplayConfidenceMap();
  void DisplayPriority();
  void DisplayData();
  void DisplayImage();

  void OpenImage(const std::string& filename);
  void OpenMask(const std::string& filename, const bool inverted);
  
  void SetupForwardLookingTable();
  void SetupTopPatchesTable();
  
  // Initialize everything.
  void Initialize();
  
  void SetupInitialIntermediateImages();
  
  // Save everything at the end of an iteration.
  void IterationComplete();
  
  // This function is called when the "Previous" or "Next" buttons are pressed, and at the end of IterationComplete().
  void ChangeDisplayedIteration();
  
  // The interactor to allow us to zoom and pan the image
  vtkSmartPointer<InteractorStyleImageNoLevel> InteractorStyle;
  
  // Track if the image has been flipped
  bool Flipped;

  // The only renderer
  vtkSmartPointer<vtkRenderer> Renderer;
  
  // Source patch outline display
  Layer UsedSourcePatchLayer;
  
  // Target patch outline display
  Layer UsedTargetPatchLayer;

  // Outline display of all forward look patches
  Layer AllForwardLookOutlinesLayer;

  // Selected forward look patch outline display
  Layer SelectedForwardLookOutlineLayer;

  // Outline display of all source patches
  Layer AllSourcePatchOutlinesLayer;

  // Selected source patch outline display
  Layer SelectedSourcePatchOutlineLayer;
  
  // Image display
  Layer ImageLayer;
  
  // Confidence on the boundary
  Layer ConfidenceLayer;
  
  // Confidence map display
  Layer ConfidenceMapLayer;
  
  // Potential patch image display
  UnsignedCharScalarImageType::Pointer PotentialTargetPatchesImage;
  
  Layer PotentialPatchesLayer;
  
  // Priority image display
  Layer PriorityLayer;
  
  // Boundary image display
  Layer BoundaryLayer;

  // Mask image display
  Layer MaskLayer;
  
  // Data image display (this is the "Data" term that is later combined with the Confidence to compute the Priority)
  Layer DataLayer;
  
  // Isophote display
  VectorLayer IsophoteLayer;

  // Boundary normals display
  VectorLayer BoundaryNormalsLayer;
  
  // The image that the user loads
  FloatVectorImageType::Pointer UserImage;
  
  // The mask that the user loads
  Mask::Pointer UserMaskImage;
  
  // The class that does all the work.
  CriminisiInpainting Inpainting;
  
  // Perform the long inpainting operation in this thread so that the UI remains active.
  ProgressThread ComputationThread;

  // A flag that determines if intermediate images should be output to files.
  bool DebugImages;
  
  // A flag that determines if debugging messages should be output.
  bool DebugMessages;

  // Output the message only if the Debug member is set to true
  void DebugMessage(const std::string&);

  // Output the message and value only if the Debug member is set to true
  template <typename T>
  void DebugMessage(const std::string& message, const T value);

  // If IterationToDisplay == 0, then we are just displaying the initial images.
  unsigned int IterationToDisplay;
  unsigned int ForwardLookToDisplay;
  unsigned int SourcePatchToDisplay;
  
  // Display zoomed in versions of the patches used at the current iteration
  void DisplayUsedPatches();
  void DisplaySourcePatch();
  void DisplayTargetPatch();
  void DisplayResultPatch();
  
  // Display the text information (scores, etc) of the patches used at the current information
  void DisplayUsedPatchInformation();
  
  // Display outlines of where the source patch came from and the target patch to which it was copied
  void HighlightUsedPatches();

  // Display outlines of the forward look target patches
  void HighlightForwardLookPatches();

  // Display the outline of the selected forward look target patch
  void HighlightSelectedForwardLookPatch();
  
  // Display outlines of the source patches
  void HighlightSourcePatches();
  
  // Display the outline of the selected source patch
  void HighlightSelectedSourcePatch();
  
  // Get the potential target patches from this iteration and outline them on a blank image.
  void CreatePotentialTargetPatchesImage();

  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  QGraphicsScene* ResultPatchScene;
  
  void OutputPairs(const std::vector<PatchPair>& patchPairs, const std::string& filename);

  // Store the candidate forward look patches from every iteration for visualization.
  // The outer vector is the iteration, and the inner vector is the look ahead patch.
  std::vector<std::vector<CandidatePairs> > AllPotentialCandidatePairs;
  
  std::vector<bool> Recorded;
  
  // These are the state of the completion at every step. The index represents the image AFTER the index'th step.
  // That is, the image at index 0 is the image after 0 iterations (the original image). At index 1 is the image after the first target region has been filled, etc.
  std::vector<InpaintingVisualizationStack> IntermediateImages;
  
  // Colors
  void SetupColors();
  QColor UsedTargetPatchColor;
  QColor UsedSourcePatchColor;
  QColor AllForwardLookPatchColor;
  QColor SelectedForwardLookPatchColor;
  QColor AllSourcePatchColor;
  QColor SelectedSourcePatchColor;
  QColor CenterPixelColor;
  QColor MaskColor;
  QColor HoleColor;
  QColor SceneBackgroundColor;
};

#include "Form.hxx"

#endif // Form_H
