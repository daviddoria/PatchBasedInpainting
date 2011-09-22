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
  Form();
  ~Form() {};
  
  // These function deal with flipping the image
  void SetCameraPosition(double leftToRight[3], double bottomToTop[3]);
  void SetCameraPosition1();
  void SetCameraPosition2();
  
  void Refresh();
  
public slots:
  
  void on_btnPrevious_clicked();
  void on_btnNext_clicked();
  
  void on_actionOpenImage_activated();
  void on_actionOpenMask_activated();
  void on_actionOpenMaskInverted_activated();
  void on_actionSaveResult_activated();
  
  void on_chkHighlightUsedPatches_clicked();
  
  void on_chkDebugImages_clicked();
  void on_chkDebugMessages_clicked();
  
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
  
  void on_btnInpaint_clicked();
  void on_btnStep_clicked();
  void on_btnInitialize_clicked();
  void on_btnStop_clicked();
  void on_btnReset_clicked();
  
  void on_actionHelp_activated();
  void on_actionQuit_activated();
  
  void on_actionFlipImage_activated();
  
  void StartProgressSlot();
  void StopProgressSlot();
  
  void RefreshSlot();
  
  void IterationCompleteSlot();
  
  void ExtractIsophotesForDisplay();
protected:
  
  void DisplayBoundaryNormals();
  void DisplayMask();
  void DisplayConfidence();
  void DisplayPriority();
  void DisplayData();

  void Initialize();
  void IterationComplete();
  void ChangeDisplayedIteration();
  
  // The interactor to allow us to zoom and pan the image
  vtkSmartPointer<InteractorStyleImageNoLevel> InteractorStyle;
  
  // Track if the image has been flipped
  bool Flipped;

  vtkSmartPointer<vtkRenderer> Renderer;
  
  // Source patch display
  Layer SourcePatchLayer;
  
  // Target patch display
  Layer TargetPatchLayer;
  
  // Image display
  Layer ImageLayer;
  
  // Confidence on the boundary
  Layer ConfidenceLayer;
  
  // Confidence map display
  Layer ConfidenceMapLayer;
  
  // Potential patch image display
  UnsignedCharScalarImageType::Pointer PotentialPatchImage;
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
  
  // The data that the user loads
  FloatVectorImageType::Pointer UserImage;
  Mask::Pointer UserMaskImage;
  
  CriminisiInpainting Inpainting;
  
  ProgressThread ComputationThread;

  bool DebugImages;
  bool DebugMessages;

  // Output the message only if the Debug member is set to true
  void DebugMessage(const std::string&);

  // Output the message and value only if the Debug member is set to true
  template <typename T>
  void DebugMessage(const std::string& message, const T value);

  // This is not unsigned because we start at -1, indicating there is no patch to display
  int CurrentUsedPatchDisplayed;
  
  void DisplayUsedPatches();
  void DisplayUsedPatchInformation();
  
  void HighlightUsedPatches();
  void DrawPotentialPatches();
  
  static const unsigned char Green[3];
  static const unsigned char Red[3];

  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  
  QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx);
  
  void OutputPairs(const std::vector<PatchPair>& patchPairs, const std::string& filename);
  
  // These are the state of the completion at every step. The index represents the image AFTER the index'th step.
  // That is, the image at index 0 is the image after 0 iterations (the original image). At index 1 is the image after the first target region has been filled, etc.
  std::vector<FloatVectorImageType::Pointer> IntermediateImages;
  std::vector<Mask::Pointer> IntermediateMaskImages;
};

#include "Form.hxx"

#endif // Form_H
