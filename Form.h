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
#include "ProgressThread.h"
#include "Types.h"

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
  
  void on_chkDebugImages_clicked();
  void on_chkDebugMessages_clicked();
  
  void on_chkImage_clicked();
  void on_chkMask_clicked();
  void on_chkPriority_clicked();
  void on_chkConfidence_clicked();
  void on_chkBoundary_clicked();
  void on_chkIsophotes_clicked();
  void on_chkData_clicked();
  void on_chkBoundaryNormals_clicked();
  
  void on_btnInpaint_clicked();
  void on_btnStop_clicked();
  void on_btnReset_clicked();
  
  void on_actionHelp_activated();
  void on_actionQuit_activated();
  
  void on_actionFlipImage_activated();
  
  void StartProgressSlot();
  void StopProgressSlot();
  
  void RefreshSlot();
  
  void IterationCompleteSlot();
  
protected:
  
  // The interactor to allow us to zoom and pan the image
  vtkSmartPointer<InteractorStyleImageNoLevel> InteractorStyle;
  
  // Track if the image has been flipped
  bool Flipped;

  vtkSmartPointer<vtkRenderer> Renderer;
  
  // Source patch display
  vtkSmartPointer<vtkImageData> SourcePatch;
  vtkSmartPointer<vtkImageSlice> SourcePatchSlice;
  vtkSmartPointer<vtkImageSliceMapper> SourcePatchSliceMapper;
  
  // Target patch display
  vtkSmartPointer<vtkImageData> TargetPatch;
  vtkSmartPointer<vtkImageSlice> TargetPatchSlice;
  vtkSmartPointer<vtkImageSliceMapper> TargetPatchSliceMapper;
  
  // Image display
  vtkSmartPointer<vtkImageData> VTKImage;
  vtkSmartPointer<vtkImageSlice> ImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> ImageSliceMapper;
  
  // Confidence image display
  vtkSmartPointer<vtkImageData> VTKConfidenceImage;
  vtkSmartPointer<vtkImageSlice> ConfidenceImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> ConfidenceImageSliceMapper;

  // Priority image display
  vtkSmartPointer<vtkImageData> VTKPriorityImage;
  vtkSmartPointer<vtkImageSlice> PriorityImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> PriorityImageSliceMapper;
  
  // Boundary image display
  vtkSmartPointer<vtkImageData> VTKBoundaryImage;
  vtkSmartPointer<vtkImageSlice> BoundaryImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> BoundaryImageSliceMapper;

  // Mask image display
  vtkSmartPointer<vtkImageData> VTKMaskImage;
  vtkSmartPointer<vtkImageSlice> MaskImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> MaskImageSliceMapper;
  
  // Data image display (this is the "Data" term that is later combined with the Confidence to compute the Priority)
  vtkSmartPointer<vtkImageData> VTKDataImage;
  vtkSmartPointer<vtkImageSlice> DataImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> DataImageSliceMapper;

  // Isophote display
  vtkSmartPointer<vtkPolyDataMapper> IsophoteMapper;
  vtkSmartPointer<vtkActor> IsophoteActor;
  vtkSmartPointer<vtkImageData> VTKIsophoteImage;
  vtkSmartPointer<vtkGlyph2D> IsophoteGlyph;

  // Boundary normals display
  vtkSmartPointer<vtkPolyDataMapper> BoundaryNormalsMapper;
  vtkSmartPointer<vtkActor> BoundaryNormalsActor;
  vtkSmartPointer<vtkImageData> VTKBoundaryNormalsImage;
  vtkSmartPointer<vtkGlyph2D> BoundaryNormalsGlyph;
  
  // The data that the user loads
  FloatVectorImageType::Pointer Image;
  Mask::Pointer MaskImage;
  
  CriminisiInpainting Inpainting;
  
  ProgressThread ComputationThread;

  bool DebugImages;
  bool DebugMessages;

  // Output the message only if the Debug member is set to true
  void DebugMessage(const std::string&);

  // Output the message and value only if the Debug member is set to true
  template <typename T>
  void DebugMessage(const std::string& message, T value);

  // This is not unsigned because we start at -1, indicating there is no patch to display
  int CurrentUsedPatchDisplayed;
  
  void DisplayUsedPatches();
  
  
  static const unsigned char Green[3];
  static const unsigned char Red[3];
  
};

#include "Form.hxx"

#endif // Form_H
