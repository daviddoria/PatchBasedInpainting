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

#include "PatchBasedInpaintingGUI.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QButtonGroup>
#include <QFileDialog>
#include <QIcon>
#include <QTextEdit>
#include <QIntValidator>

// VTK
#include <vtkActor.h>
#include <vtkArrowSource.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkGlyph2D.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkImageSliceMapper.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLImageDataWriter.h> // For debugging only

// Boost
#include <boost/bind.hpp>

// Custom
#include "FileSelector.h"
#include "Helpers.h"
#include "HelpersDisplay.h"
#include "HelpersOutput.h"
#include "HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"
#include "Mask.h"
#include "PixmapDelegate.h"
#include "PriorityCriminisi.h"
#include "PriorityDepth.h"
#include "PriorityManual.h"
#include "PriorityOnionPeel.h"
#include "Types.h"

Q_DECLARE_METATYPE(PatchPair)

void PatchBasedInpaintingGUI::DefaultConstructor()
{
  EnterFunction("DefaultConstructor()");
  //std::cout << "DefaultConstructor()" << std::endl;
  // This function is called by both constructors. This avoid code duplication.
  this->setupUi(this);

  QButtonGroup* groupCompare = new QButtonGroup;
  groupCompare->addButton(radCompareOriginal);
  groupCompare->addButton(radCompareBlurred);
  groupCompare->addButton(radCompareCIELAB);
  radCompareOriginal->setChecked(true);
  
  QButtonGroup* groupSortBy = new QButtonGroup;
  groupSortBy->addButton(radSortByColorAndDepth);
  groupSortBy->addButton(radSortByColorDifference);
  groupSortBy->addButton(radSortByDepthDifference);
  groupSortBy->addButton(radSortByFullDifference);
  radSortByFullDifference->setChecked(true);
  
  this->PatchCompare = new SelfPatchCompare;

  this->PatchRadius = 10;
  this->NumberOfTopPatchesToSave = 0;
  this->NumberOfForwardLook = 0;
  this->GoToIteration = 0;
  this->NumberOfTopPatchesToDisplay = 0;

  this->CameraLeftToRightVector.resize(3);
  this->CameraLeftToRightVector[0] = -1;
  this->CameraLeftToRightVector[1] = 0;
  this->CameraLeftToRightVector[2] = 0;

  this->CameraBottomToTopVector.resize(3);
  this->CameraBottomToTopVector[0] = 0;
  this->CameraBottomToTopVector[1] = 1;
  this->CameraBottomToTopVector[2] = 0;

  this->PatchDisplaySize = 100;

  SetupColors();

  SetCheckboxVisibility(false);

  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(this->SceneBackgroundColor);

  this->TargetPatchScene = new QGraphicsScene();
  this->TargetPatchScene->setBackgroundBrush(brush);
  this->gfxTarget->setScene(TargetPatchScene);

  this->SourcePatchScene = new QGraphicsScene();
  this->SourcePatchScene->setBackgroundBrush(brush);
  this->gfxSource->setScene(SourcePatchScene);

  this->ResultPatchScene = new QGraphicsScene();
  this->ResultPatchScene->setBackgroundBrush(brush);
  this->gfxResult->setScene(ResultPatchScene);

  this->UserPatchScene = new QGraphicsScene();
  this->UserPatchScene->setBackgroundBrush(brush);
  this->gfxUserPatch->setScene(UserPatchScene);

  this->IterationToDisplay = 0;
  this->ForwardLookToDisplay = 0;
  this->SourcePatchToDisplay = 0;

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");
  QIcon saveIcon = QIcon::fromTheme("document-save");

  // Setup toolbar
  actionOpen->setIcon(openIcon);
  this->toolBar->addAction(actionOpen);

  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);

  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();
  this->InteractorStyle->TrackballStyle->AddObserver(CustomTrackballStyle::PatchesMovedEvent, this, &PatchBasedInpaintingGUI::UserPatchMoved);

  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->UserPatchLayer.ImageSlice->SetPickable(true);
  
  this->ImageLayer.ImageSlice->SetPickable(false);
  this->BoundaryLayer.ImageSlice->SetPickable(false);
  this->PriorityLayer.ImageSlice->SetPickable(false);
  this->MaskLayer.ImageSlice->SetPickable(false);
  this->UsedTargetPatchLayer.ImageSlice->SetPickable(false);
  this->UsedSourcePatchLayer.ImageSlice->SetPickable(false);
  this->AllSourcePatchOutlinesLayer.ImageSlice->SetPickable(false);
  this->AllForwardLookOutlinesLayer.ImageSlice->SetPickable(false);
  this->SelectedForwardLookOutlineLayer.ImageSlice->SetPickable(false);
  this->SelectedSourcePatchOutlineLayer.ImageSlice->SetPickable(false);
  
  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);
  this->Renderer->AddViewProp(this->BoundaryLayer.ImageSlice);
  this->Renderer->AddViewProp(this->PriorityLayer.ImageSlice);
  this->Renderer->AddViewProp(this->MaskLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedTargetPatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedSourcePatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->AllSourcePatchOutlinesLayer.ImageSlice);
  this->Renderer->AddViewProp(this->AllForwardLookOutlinesLayer.ImageSlice);

  this->Renderer->AddViewProp(this->SelectedForwardLookOutlineLayer.ImageSlice);// This should be added after AllForwardLookOutlinesLayer.
  this->Renderer->AddViewProp(this->SelectedSourcePatchOutlineLayer.ImageSlice);// This should be added after AllSourcePatchOutlinesLayer.
  this->Renderer->AddViewProp(this->UserPatchLayer.ImageSlice);
  
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();
  
  this->UserImage = FloatVectorImageType::New();
  this->UserMaskImage = Mask::New();

  //this->Inpainting.SetPatchSearchFunctionToTwoStepDepth();
  this->Inpainting.SetPatchSearchFunctionToNormal();
  //this->Inpainting.SetDebugFunctionEnterLeave(true);
  
  SetComparisonFunctionsFromGUI();
  SetSortFunctionFromGUI();

  connect(&ComputationThread, SIGNAL(StartProgressSignal()), this, SLOT(slot_StartProgress()), Qt::QueuedConnection);
  connect(&ComputationThread, SIGNAL(StopProgressSignal()), this, SLOT(slot_StopProgress()), Qt::QueuedConnection);

  // Using a blocking connection allows everything (computation and drawing) to be performed sequentially which is helpful for debugging,
  // but makes the interface very very choppy.
  // We are assuming that the computation takes longer than the drawing.
  //connect(&ComputationThread, SIGNAL(IterationCompleteSignal()), this, SLOT(IterationCompleteSlot()), Qt::QueuedConnection);
  
  qRegisterMetaType<PatchPair>("PatchPair");
  connect(&ComputationThread, SIGNAL(IterationCompleteSignal(const PatchPair&)), this, SLOT(slot_IterationComplete(const PatchPair&)), Qt::BlockingQueuedConnection);

  connect(&ComputationThread, SIGNAL(RefreshSignal()), this, SLOT(slot_Refresh()), Qt::QueuedConnection);

  //disconnect(this->topPatchesTableWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(on_topPatchesTableWidget_currentCellChanged(int,int,int,int)), Qt::AutoConnection);
  //this->topPatchesTableWidget->disconnect(SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(on_topPatchesTableWidget_currentCellChanged(int,int,int,int)));
  //this->topPatchesTableWidget->disconnect();
  //connect(this->topPatchesTableWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(on_topPatchesTableWidget_currentCellChanged(int,int,int,int)), Qt::BlockingQueuedConnection);

  // Set the progress bar to marquee mode
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();

  this->ComputationThread.SetObject(&(this->Inpainting));

  InitializeGUIElements();

  // Setup forwardLook table
  this->ForwardLookModel = new ForwardLookTableModel(this->IterationRecords, this->ImageDisplayStyle);
  this->ForwardLookTableView->setModel(this->ForwardLookModel);
  this->ForwardLookTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  PixmapDelegate* forwardLookPixmapDelegate = new PixmapDelegate;
  this->ForwardLookTableView->setItemDelegateForColumn(0, forwardLookPixmapDelegate);

  this->connect(this->ForwardLookTableView->selectionModel(), SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )),
                SLOT(slot_ForwardLookTableView_changed(const QModelIndex & , const QModelIndex & )));

  // Setup top patches table
  this->TopPatchesModel = new TopPatchesTableModel(this->IterationRecords, this->ImageDisplayStyle);
  this->TopPatchesTableView->setModel(this->TopPatchesModel);
  this->TopPatchesTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  PixmapDelegate* topPatchesPixmapDelegate = new PixmapDelegate;
  this->TopPatchesTableView->setItemDelegateForColumn(0, topPatchesPixmapDelegate);

  this->connect(this->TopPatchesTableView->selectionModel(), SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )), SLOT(slot_TopPatchesTableView_changed(const QModelIndex & , const QModelIndex & )));

  Helpers::CreateTransparentVTKImage(Helpers::SizeFromRadius(this->PatchRadius), this->UserPatchLayer.ImageData);
  unsigned char userPatchColor[3];
  HelpersQt::QColorToUCharColor(this->UserPatchColor, userPatchColor);
  Helpers::BlankAndOutlineImage(this->UserPatchLayer.ImageData, userPatchColor);

  itk::Size<2> patchSize = Helpers::SizeFromRadius(this->PatchRadius);
  this->UserPatchRegion.SetSize(patchSize);

  this->IntValidator = new QIntValidator(0, 10000, this);
  this->txtPatchRadius->setValidator(this->IntValidator);
  this->txtNumberOfTopPatchesToSave->setValidator(this->IntValidator);
  this->txtNumberOfForwardLook->setValidator(this->IntValidator);
  this->txtGoToIteration->setValidator(this->IntValidator);
  this->txtNumberOfTopPatchesToDisplay->setValidator(this->IntValidator);

  LeaveFunction("DefaultConstructor()");
}

// Default constructor
PatchBasedInpaintingGUI::PatchBasedInpaintingGUI()
{
  DefaultConstructor();
};

PatchBasedInpaintingGUI::PatchBasedInpaintingGUI(const std::string& imageFileName, const std::string& maskFileName, const bool debugEnterLeave = false)
{
  this->SetDebugFunctionEnterLeave(debugEnterLeave);
  
  EnterFunction("PatchBasedInpaintingGUI()");

  std::cout << "Image: " << imageFileName << " Mask: " << maskFileName << std::endl;
  
  DefaultConstructor();

  OpenImage(imageFileName);
  OpenMask(maskFileName, false);
  Initialize();
  LeaveFunction("PatchBasedInpaintingGUI()");
}


void PatchBasedInpaintingGUI::UserPatchMoved()
{
  EnterFunction("UserPatchMoved()");
  // Snap user patch to integer pixels
  double position[3];
  this->UserPatchLayer.ImageSlice->GetPosition(position);
  position[0] = round(position[0]);
  position[1] = round(position[1]);
  this->UserPatchLayer.ImageSlice->SetPosition(position);
  this->qvtkWidget->GetRenderWindow()->Render();

  ComputeUserPatchRegion();

  if(this->chkDisplayUserPatch->isChecked())
    {
    DisplayUserPatch();
    }

  if(this->IterationToDisplay < 1)
    {
    LeaveFunction("UserPatchMoved()");
    return;
    }

  unsigned int iterationToCompare = this->IterationToDisplay - 1;
  SelfPatchCompare* patchCompare = new SelfPatchCompare;
  patchCompare->SetImage(this->IterationRecords[iterationToCompare].Image);
  patchCompare->SetMask(this->IterationRecords[iterationToCompare].MaskImage);
  patchCompare->SetNumberOfComponentsPerPixel(this->UserImage->GetNumberOfComponentsPerPixel());
  patchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,patchCompare,_1));
  CandidatePairs candidatePairs(this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay].TargetPatch);
  Patch userPatch(this->UserPatchRegion);
  candidatePairs.AddPairFromPatch(userPatch);
  patchCompare->SetPairs(&candidatePairs);
  patchCompare->ComputeAllSourceDifferences();

  std::stringstream ss;
  ss << candidatePairs[0].GetAverageAbsoluteDifference();
  lblUserPatchError->setText(ss.str().c_str());

  LeaveFunction("UserPatchMoved()");
}

void PatchBasedInpaintingGUI::SetupColors()
{
  this->UsedTargetPatchColor = Qt::red;
  this->UsedSourcePatchColor = Qt::green;
  this->AllForwardLookPatchColor = Qt::darkCyan;
  this->SelectedForwardLookPatchColor = Qt::cyan;
  this->AllSourcePatchColor = Qt::darkMagenta;
  this->SelectedSourcePatchColor = Qt::magenta;
  this->CenterPixelColor = Qt::blue;
  this->MaskColor = Qt::darkGray;
  this->UserPatchColor = Qt::darkYellow;
  //this->HoleColor = Qt::gray;
  this->HoleColor.setRgb(255, 153, 0); // Orange
  this->SceneBackgroundColor.setRgb(153, 255, 0); // ?
}

void PatchBasedInpaintingGUI::OpenMask(const std::string& fileName, const bool inverted)
{
  //std::cout << "OpenMask()" << std::endl;
  typedef itk::ImageFileReader<Mask> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  Helpers::DeepCopy<Mask>(reader->GetOutput(), this->UserMaskImage);
  //std::cout << "UserMaskImage region: " << this->UserMaskImage->GetLargestPossibleRegion() << std::endl;
  
  // For this program, we ALWAYS assume the hole to be filled is white, and the valid/source region is black.
  // This is not simply reversible because of some subtle erosion operations that are performed.
  // For this reason, we provide an "load inverted mask" action in the file menu.
  this->UserMaskImage->SetValidValue(0);
  this->UserMaskImage->SetHoleValue(255);

  this->statusBar()->showMessage("Opened mask.");

  this->UserMaskImage->Cleanup();

  if(inverted)
    {
    this->UserMaskImage->Invert();
    }

  //Helpers::DebugWriteImageConditional<Mask>(this->UserMaskImage, "Debug/InvertedMask.png", this->DebugImages);
}


void PatchBasedInpaintingGUI::OpenImage(const std::string& fileName)
{
  //std::cout << "OpenImage()" << std::endl;
  /*
  // The non static version of the above is something like this:
  QFileDialog myDialog;
  QDir fileFilter("Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");
  myDialog.setFilter(fileFilter);
  QString fileName = myDialog.exec();
  */

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  // If the image doesn't have at least 3 channels, it cannot be displayed as a color image.
  if(reader->GetOutput()->GetNumberOfComponentsPerPixel() < 3)
    {
    this->radDisplayMagnitudeImage->setChecked(true);
    }
  this->spinChannelToDisplay->setMaximum(reader->GetOutput()->GetNumberOfComponentsPerPixel() - 1);

  //this->Image = reader->GetOutput();
  
  Helpers::DeepCopy<FloatVectorImageType>(reader->GetOutput(), this->UserImage);
  
  //std::cout << "UserImage region: " << this->UserImage->GetLargestPossibleRegion() << std::endl;

  HelpersDisplay::ITKVectorImageToVTKImage(this->UserImage, this->ImageLayer.ImageData, this->ImageDisplayStyle);

  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();

  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);

  this->AllForwardLookOutlinesLayer.ImageData->SetDimensions(this->UserImage->GetLargestPossibleRegion().GetSize()[0],
                                                             this->UserImage->GetLargestPossibleRegion().GetSize()[1], 1);
  this->AllForwardLookOutlinesLayer.ImageData->AllocateScalars();
  this->AllSourcePatchOutlinesLayer.ImageData->SetDimensions(this->UserImage->GetLargestPossibleRegion().GetSize()[0],
                                                             this->UserImage->GetLargestPossibleRegion().GetSize()[1], 1);
  this->AllSourcePatchOutlinesLayer.ImageData->AllocateScalars();

}

void PatchBasedInpaintingGUI::Reset()
{
  this->txtNumberOfForwardLook->setEnabled(true);
  this->txtNumberOfTopPatchesToSave->setEnabled(true);
  this->btnInpaint->setEnabled(false);
  this->btnStep->setEnabled(false);
  this->btnInitialize->setEnabled(true);
  this->btnReset->setEnabled(false);
  this->txtPatchRadius->setEnabled(true);
  
  this->IterationRecords.clear();
  Initialize();
  Refresh();
}

void PatchBasedInpaintingGUI::DisplayMask()
{
  //vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  //Helpers::ITKScalarImageToScaledVTKImage<Mask>(this->IntermediateImages[this->IterationToDisplay].MaskImage, temp);  
  //Helpers::MakeValidPixelsTransparent(temp, this->MaskLayer.ImageData, 0); // Set the zero pixels of the mask to transparent
  
  this->IterationRecords[this->IterationToDisplay].MaskImage->MakeVTKImage(this->MaskLayer.ImageData, QColor(Qt::white), this->HoleColor, false, true); // (..., holeTransparent, validTransparent);
  this->qvtkWidget->GetRenderWindow()->Render();
}

void PatchBasedInpaintingGUI::ComputeUserPatchRegion()
{
  double position[3];
  this->UserPatchLayer.ImageSlice->GetPosition(position);
  itk::Index<2> positionIndex;
  positionIndex[0] = position[0];
  positionIndex[1] = position[1];
  this->UserPatchRegion.SetIndex(positionIndex);

  itk::Size<2> patchSize = Helpers::SizeFromRadius(this->PatchRadius);
  this->UserPatchRegion.SetSize(patchSize);
}

void PatchBasedInpaintingGUI::DisplayUserPatch()
{
  EnterFunction("DisplayUserPatch");

  ComputeUserPatchRegion();
  QImage userPatch = HelpersQt::GetQImage<FloatVectorImageType>(this->IterationRecords[this->IterationToDisplay].Image,
                                                                this->UserPatchRegion, this->ImageDisplayStyle);
  userPatch = HelpersQt::FitToGraphicsView(userPatch, gfxTarget);
  this->UserPatchScene->addPixmap(QPixmap::fromImage(userPatch));

  LeaveFunction("DisplayUserPatch");
}

void PatchBasedInpaintingGUI::DisplayImage()
{
  EnterFunction("DisplayImage");
  HelpersDisplay::ITKVectorImageToVTKImage(this->IterationRecords[this->IterationToDisplay].Image, this->ImageLayer.ImageData, this->ImageDisplayStyle);

  this->qvtkWidget->GetRenderWindow()->Render();
  LeaveFunction("DisplayImage");
}

void PatchBasedInpaintingGUI::DisplayBoundary()
{
  EnterFunction("DisplayBoundary");
  Helpers::ITKScalarImageToScaledVTKImage<UnsignedCharScalarImageType>(this->IterationRecords[this->IterationToDisplay].Boundary, this->BoundaryLayer.ImageData);
  this->qvtkWidget->GetRenderWindow()->Render();
  LeaveFunction("DisplayBoundary");
}

void PatchBasedInpaintingGUI::DisplayPriority()
{
  EnterFunction("DisplayPriority");
  vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->IterationRecords[this->IterationToDisplay].Priority, temp);
  Helpers::MakeValueTransparent(temp, this->PriorityLayer.ImageData, 0); // Set the zero pixels to transparent
  this->qvtkWidget->GetRenderWindow()->Render();
  LeaveFunction("DisplayPriority");
}

void PatchBasedInpaintingGUI::RefreshVTK()
{
  EnterFunction("RefreshVTK()");
  try
  {

    // The following are valid for all iterations
    if(this->chkDisplayUserPatch->isChecked())
      {
      DisplayUserPatch();
      }

    if(this->chkImage->isChecked())
      {
      DisplayImage();
      }

    if(this->chkMask->isChecked())
      {
      DisplayMask();
      }

    if(this->chkPriority->isChecked())
      {
      DisplayPriority();
      }

    if(this->chkBoundary->isChecked())
      {
      DisplayBoundary();
      }

    // The following are only valid for iterations after 0
    if(this->IterationToDisplay > 0)
      {
      this->UsedSourcePatchLayer.ImageSlice->SetVisibility(this->chkHighlightUsedPatches->isChecked());
      this->UsedTargetPatchLayer.ImageSlice->SetVisibility(this->chkHighlightUsedPatches->isChecked());

      this->SelectedForwardLookOutlineLayer.ImageSlice->SetVisibility(this->chkDisplayForwardLookPatchLocations->isChecked());
      this->AllForwardLookOutlinesLayer.ImageSlice->SetVisibility(this->chkDisplayForwardLookPatchLocations->isChecked());
      if(this->chkDisplayForwardLookPatchLocations->isChecked())
        {
        HighlightForwardLookPatches();
        }

      this->SelectedSourcePatchOutlineLayer.ImageSlice->SetVisibility(this->chkDisplaySourcePatchLocations->isChecked());
      this->AllSourcePatchOutlinesLayer.ImageSlice->SetVisibility(this->chkDisplaySourcePatchLocations->isChecked());
      if(this->chkDisplaySourcePatchLocations->isChecked())
        {
        HighlightSourcePatches();
        }
      }
    else
      {
      this->UsedSourcePatchLayer.ImageSlice->SetVisibility(false);
      this->UsedTargetPatchLayer.ImageSlice->SetVisibility(false);
      this->SelectedForwardLookOutlineLayer.ImageSlice->SetVisibility(false);
      this->AllForwardLookOutlinesLayer.ImageSlice->SetVisibility(false);
      this->SelectedSourcePatchOutlineLayer.ImageSlice->SetVisibility(false);
      this->AllSourcePatchOutlinesLayer.ImageSlice->SetVisibility(false);
      }

    this->qvtkWidget->GetRenderWindow()->Render();
    LeaveFunction("RefreshVTK()");
    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in Refresh!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::RefreshQt()
{
  EnterFunction("RefreshQt()");
  
  ChangeDisplayedTopPatch();
  ChangeDisplayedForwardLookPatch();
  SetupForwardLookingTable();
  SetupTopPatchesTable();

  LeaveFunction("RefreshQt()");
}

void PatchBasedInpaintingGUI::Refresh()
{
  EnterFunction("Refresh()");
  RefreshVTK();
  RefreshQt();
  LeaveFunction("Refresh()");
}


void PatchBasedInpaintingGUI::Initialize()
{
  EnterFunction("PatchBasedInpaintingGUI::Initialize()");
  // Reset some things (this is so that if we want to run another completion it will work normally)

  // Color the pixels inside the hole in the image so we will notice if they are erroneously being copied/used.
  this->UserMaskImage->ApplyToVectorImage<FloatVectorImageType>(this->UserImage, this->HoleColor);

  // Provide required data.
  this->Inpainting.SetPatchRadius(this->PatchRadius);
  this->Inpainting.SetMask(this->UserMaskImage);
  this->Inpainting.SetImage(this->UserImage);
  
  // The PatchSortFunction has already been set by the radio buttons.
  
  std::cout << "User Image: " << this->UserImage->GetLargestPossibleRegion().GetSize() << std::endl;
  std::cout << "User Mask: " << this->UserMaskImage->GetLargestPossibleRegion().GetSize() << std::endl;
  HelpersOutput::WriteImage<Mask>(this->UserMaskImage, "mask.mha");

  // Setup verbosity.
  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->Inpainting.SetDebugMessages(this->chkDebugMessages->isChecked());
  //this->Inpainting.SetDebugFunctionEnterLeave(false);
  
  // Setup the priority function
  
  //this->Inpainting.SetPriorityFunction<PriorityOnionPeel>();
  //this->Inpainting.SetPriorityFunction<PriorityCriminisi>();
  //this->Inpainting.SetPriorityFunction<PriorityDepth>();
  this->Inpainting.SetPriorityFunction<PriorityManual>();
  
  UnsignedCharScalarImageType::Pointer manualPriorityImage = UnsignedCharScalarImageType::New();
  std::string manualPriorityImageFileName = "/media/portable/Data/LidarImageCompletion/PaperDataSets/trashcan/trashcan_medium/trashcan_manualPriority.png";
  Helpers::ReadImage<UnsignedCharScalarImageType>(manualPriorityImageFileName, manualPriorityImage);
  
  reinterpret_cast<PriorityManual*>(this->Inpainting.GetPriorityFunction())->SetManualPriorityImage(manualPriorityImage);
  //this->Inpainting.GetPriorityFunction()->SetDebugFunctionEnterLeave(true);
  
  // Setup the patch comparison function
  //SelfPatchCompare* patchCompare = new SelfPatchCompare;
  this->PatchCompare->SetNumberOfComponentsPerPixel(this->UserImage->GetNumberOfComponentsPerPixel());
  this->Inpainting.SetPatchCompare(this->PatchCompare);

  // Setup the sorting function
  this->Inpainting.PatchSortFunction = &SortByAverageAbsoluteDifference;
  
  // Finish initializing
  this->Inpainting.Initialize();

  SetupInitialIntermediateImages();
  this->IterationToDisplay = 0;
  ChangeDisplayedIteration();
  
  SetCheckboxVisibility(true);

  Refresh();
  LeaveFunction("PatchBasedInpaintingGUI::Initialize()");
}

void PatchBasedInpaintingGUI::DisplaySourcePatch()
{
  try
  {
    EnterFunction("DisplaySourcePatch()");

    if(this->IterationToDisplay < 1)
      {
      std::cerr << "Can only display result patch for iterations > 0." << std::endl;
      LeaveFunction("DisplaySourcePatch()");
      return;
      }

    FloatVectorImageType::Pointer currentImage = this->IterationRecords[this->IterationToDisplay].Image;

    // This -1 is because the 0th iteration is the initial condition.
//     std::cout << "this->IterationRecords: " << this->IterationRecords.size()
//               << " PotentialPairSets: " << this->IterationRecords[this->IterationToDisplay].PotentialPairSets.size()
//               << " Iteration: " << this->IterationToDisplay << " ForwardLook: " << this->ForwardLookToDisplay << std::endl;
    const CandidatePairs& candidatePairs = this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay];

    QImage sourceImage = HelpersQt::GetQImage<FloatVectorImageType>(currentImage, candidatePairs[this->SourcePatchToDisplay].SourcePatch.Region, this->ImageDisplayStyle);
    sourceImage = HelpersQt::FitToGraphicsView(sourceImage, gfxTarget);
    this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));
    LeaveFunction("DisplaySourcePatch()");
    //Refresh();
    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DisplaySourcePatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::DisplayTargetPatch()
{
  try
  {
    EnterFunction("DisplayTargetPatch()");
    
    if(this->IterationToDisplay < 1)
      {
      std::cerr << "Can only display target patch for iterations > 0." << std::endl;
      LeaveFunction("DisplayTargetPatch()");
      return;
      }

    FloatVectorImageType::Pointer currentImage = this->IterationRecords[this->IterationToDisplay].Image;

    const CandidatePairs& candidatePairs = this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay];

    // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration
    // (as the current mask has been cleared where the target patch was copied).
    Mask::Pointer currentMask = this->IterationRecords[this->IterationToDisplay].MaskImage;

    // Target
    QImage targetImage = HelpersQt::GetQImage<FloatVectorImageType>(currentImage, candidatePairs.TargetPatch.Region, this->ImageDisplayStyle);

    targetImage = HelpersQt::FitToGraphicsView(targetImage, gfxTarget);
    this->TargetPatchScene->addPixmap(QPixmap::fromImage(targetImage));
    LeaveFunction("DisplayTargetPatch()");
    //Refresh();
    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DisplayTargetPatch!" << std::endl;
    std::cout << "Trying to display patch " << this->ForwardLookToDisplay << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::DisplayResultPatch()
{
  try
  {
    EnterFunction("DisplayResultPatch()");
    
    if(this->IterationToDisplay < 1)
      {
      std::cerr << "Can only display result patch for iterations > 0." << std::endl;
      return;
      }

    FloatVectorImageType::Pointer currentImage = this->IterationRecords[this->IterationToDisplay].Image;
    
    const CandidatePairs& candidatePairs = this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay];

    const PatchPair& patchPair = candidatePairs[this->SourcePatchToDisplay];
    
    // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration
    // (as the current mask has been cleared where the target patch was copied).
    Mask::Pointer currentMask = this->IterationRecords[this->IterationToDisplay].MaskImage;

    itk::Size<2> regionSize = patchPair.SourcePatch.Region.GetSize(); // this could equally as well be TargetPatch
    
    QImage qimage(regionSize[0], regionSize[1], QImage::Format_RGB888);

    itk::ImageRegionIterator<FloatVectorImageType> sourceIterator(currentImage, patchPair.SourcePatch.Region);
    itk::ImageRegionIterator<FloatVectorImageType> targetIterator(currentImage, patchPair.TargetPatch.Region);
    itk::ImageRegionIterator<Mask> maskIterator(currentMask, patchPair.TargetPatch.Region);

    FloatVectorImageType::Pointer resultPatch = FloatVectorImageType::New();
    resultPatch->SetNumberOfComponentsPerPixel(currentImage->GetNumberOfComponentsPerPixel());
    itk::Size<2> patchSize = Helpers::SizeFromRadius(this->PatchRadius);
    itk::ImageRegion<2> region;
    region.SetSize(patchSize);
    resultPatch->SetRegions(region);
    resultPatch->Allocate();
    
    while(!maskIterator.IsAtEnd())
      {
      FloatVectorImageType::PixelType pixel;
    
      if(currentMask->IsHole(maskIterator.GetIndex()))
	{
	pixel = sourceIterator.Get();
	}
      else
	{
	pixel = targetIterator.Get();
	}
      
      itk::Offset<2> offset = sourceIterator.GetIndex() - patchPair.SourcePatch.Region.GetIndex();
      itk::Index<2> offsetIndex;
      offsetIndex[0] = offset[0];
      offsetIndex[1] = offset[1];
      resultPatch->SetPixel(offsetIndex, pixel);

      ++sourceIterator;
      ++targetIterator;
      ++maskIterator;
      }

    // Color the center pixel
    //qimage.setPixel(regionSize[0]/2, regionSize[1]/2, this->CenterPixelColor.rgb());

    qimage = HelpersQt::GetQImage<FloatVectorImageType>(resultPatch, resultPatch->GetLargestPossibleRegion(), this->ImageDisplayStyle);

    qimage = HelpersQt::FitToGraphicsView(qimage, gfxResult);
    this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));
    LeaveFunction("DisplayResultPatch()");
    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DisplayResultPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::DisplayUsedPatches()
{
  EnterFunction("DisplayUsedPatches()");

  try
  {
    // There are no patches used in the 0th iteration (initial conditions) so it doesn't make sense to display them.
    // Instead we display blank images.
    if(this->IterationToDisplay < 1)
      {
      this->TargetPatchScene->clear();
      this->SourcePatchScene->clear();

      return;
      }
      
    DisplaySourcePatch();
    DisplayTargetPatch();
    DisplayResultPatch();
    Refresh();
    LeaveFunction("DisplayUsedPatches()");
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DisplayUsedPatches!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::HighlightForwardLookPatches()
{
  EnterFunction("HighlightForwardLookPatches()");
  try
  {
    // Delete any current highlight patches. We want to delete these (if they exist) no matter what because
    // then they won't be displayed if the box is not checked (they will respect the check box).
    Helpers::BlankImage(this->AllForwardLookOutlinesLayer.ImageData);

    // If the user has not requested to display the patches, quit.
    if(!this->chkDisplayForwardLookPatchLocations->isChecked())
      {
      std::cout << "HighlightForwardLookPatches: chkDisplayForwardLookPatchLocations not checked!" << std::endl;
      return;
      }

    if(this->IterationToDisplay < 1)
      {
      std::cout << "HighlightForwardLookPatches: IterationToDisplay < 1!" << std::endl;
      return;
      }

    // Get the candidate patches and make sure we have requested a valid set.
    const std::vector<CandidatePairs>& candidatePairs = this->IterationRecords[this->IterationToDisplay].PotentialPairSets;

    unsigned char borderColor[3];
    HelpersQt::QColorToUCharColor(this->AllForwardLookPatchColor, borderColor);
    unsigned char centerPixelColor[3];
    HelpersQt::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);
    
    for(unsigned int candidateId = 0; candidateId < candidatePairs.size(); ++candidateId)
      {
      const Patch& currentPatch = candidatePairs[candidateId].TargetPatch;
      std::cout << "Outlining " << currentPatch.Region << std::endl;
      //DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);
      
      Helpers::BlankAndOutlineRegion(this->AllForwardLookOutlinesLayer.ImageData, currentPatch.Region, borderColor);
      
      Helpers::SetRegionCenterPixel(this->AllForwardLookOutlinesLayer.ImageData, currentPatch.Region, centerPixelColor);
      }
    
    std::cout << "Highlighted " << candidatePairs.size() << " forward look patches." << std::endl;
    
//     if(this->forwardLookingTableWidget->currentRow() < 0)
//       {
//       this->forwardLookingTableWidget->selectRow(0);
//       }
    HighlightSelectedForwardLookPatch();

    this->qvtkWidget->GetRenderWindow()->Render();
    LeaveFunction("HighlightForwardLookPatches()");

    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in HighlightForwardLookPatches!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void PatchBasedInpaintingGUI::HighlightSourcePatches()
{
  try
  {
    EnterFunction("HighlightSourcePatches()");

    // Delete any current highlight patches. We want to delete these (if they exist) no matter what because then they won't be displayed if the box is not checked (they will respect the check box).
    Helpers::BlankImage(this->AllSourcePatchOutlinesLayer.ImageData);

    if(!this->chkDisplaySourcePatchLocations->isChecked())
      {
      LeaveFunction("HighlightSourcePatches()");
      return;
      }

    if(this->IterationToDisplay < 1)
      {
      LeaveFunction("HighlightSourcePatches()");
      return;
      }

    const CandidatePairs& candidatePairs = this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay];

    unsigned char borderColor[3];
    HelpersQt::QColorToUCharColor(this->AllSourcePatchColor, borderColor);
    unsigned char centerPixelColor[3];
    HelpersQt::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);

    unsigned int numberToDisplay = std::min(candidatePairs.size(), NumberOfTopPatchesToDisplay);
    for(unsigned int candidateId = 0; candidateId < numberToDisplay; ++candidateId)
      {
      //DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);
      const Patch& currentPatch = candidatePairs[candidateId].SourcePatch;
      Helpers::BlankAndOutlineRegion(this->AllSourcePatchOutlinesLayer.ImageData, currentPatch.Region, borderColor);
      Helpers::SetRegionCenterPixel(this->AllSourcePatchOutlinesLayer.ImageData, currentPatch.Region, centerPixelColor);
      }

    //std::cout << "Selected forward look patch should be: " << this->forwardLookingTableWidget->currentRow() << std::endl;
//     if(this->topPatchesTableWidget->currentRow() < 0)
//       {
//       this->topPatchesTableWidget->selectRow(0);
//       }
    
    this->qvtkWidget->GetRenderWindow()->Render();
    LeaveFunction("HighlightSourcePatches()");
    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in HighlightSourcePatches!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::HighlightUsedPatches()
{
  try
  {
    EnterFunction("HighlightUsedPatches()");
    if(this->IterationToDisplay < 1)
      {
      std::cerr << "Can only display used patches for iterations >= 1" << std::endl;
      LeaveFunction("HighlightUsedPatches()");
      return;
      }
    PatchPair patchPair = this->IterationRecords[this->IterationToDisplay].UsedPatchPair;

    unsigned char centerPixelColor[3];
    HelpersQt::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);
    
    // Target
    DebugMessage("Target...");
    Patch targetPatch = patchPair.TargetPatch;

    unsigned int patchSize = targetPatch.Region.GetSize()[0];
    //std::cout << "Displaying used target patch " << this->CurrentUsedPatchDisplayed << " : " << targetPatch.Region << std::endl;
    DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);
    this->UsedTargetPatchLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
    unsigned char targetPatchColor[3];
    HelpersQt::QColorToUCharColor(this->UsedTargetPatchColor, targetPatchColor);
    Helpers::BlankAndOutlineImage(this->UsedTargetPatchLayer.ImageData, targetPatchColor);
    Helpers::SetImageCenterPixel(this->UsedTargetPatchLayer.ImageData, centerPixelColor);
    this->UsedTargetPatchLayer.ImageSlice->SetPosition(targetPatch.Region.GetIndex()[0], targetPatch.Region.GetIndex()[1], 0);

    // Source
    DebugMessage("Source...");
    Patch sourcePatch = patchPair.SourcePatch;

    //std::cout << "Displaying used source patch " << this->CurrentUsedPatchDisplayed << " : " << sourcePatch.Region << std::endl;
    DebugMessage<itk::ImageRegion<2> >("Source patch region: ", sourcePatch.Region);
    this->UsedSourcePatchLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
    unsigned char sourcePatchColor[3];
    HelpersQt::QColorToUCharColor(this->UsedSourcePatchColor, sourcePatchColor);
    Helpers::BlankAndOutlineImage(this->UsedSourcePatchLayer.ImageData, sourcePatchColor);
    Helpers::SetImageCenterPixel(this->UsedSourcePatchLayer.ImageData, centerPixelColor);
    this->UsedSourcePatchLayer.ImageSlice->SetPosition(sourcePatch.Region.GetIndex()[0], sourcePatch.Region.GetIndex()[1], 0);

    Refresh();
    LeaveFunction("HighlightUsedPatches()");
    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in HighlightUsedPatches!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::DisplayUsedPatchInformation()
{
  try
  {
    EnterFunction("DisplayUsedPatchInformation()");

    if(this->IterationToDisplay < 1)
      {
      std::cerr << "Can only display used patch information for iterations >= 1" << std::endl;
      return;
      }
    this->ForwardLookToDisplay = 0;
    this->SourcePatchToDisplay = 0;
    
    SetupForwardLookingTable();
    SetupTopPatchesTable();
    
    ChangeDisplayedForwardLookPatch();
    ChangeDisplayedTopPatch();

    PatchPair patchPair = this->IterationRecords[this->IterationToDisplay].UsedPatchPair;

    // Source information
    /*
    std::stringstream ssSource;
    ssSource << "(" << patchPair.SourcePatch.Region.GetIndex()[0] << ", " << patchPair.SourcePatch.Region.GetIndex()[1] << ")";
    this->lblSourceCorner->setText(ssSource.str().c_str());
    
    // Target information
    std::stringstream ssTarget;
    ssTarget << "(" << patchPair.TargetPatch.Region.GetIndex()[0] << ", " << patchPair.TargetPatch.Region.GetIndex()[1] << ")";
    this->lblTargetCorner->setText(ssTarget.str().c_str());
    */
    
    /*
    // Patch pair information
    float ssd = patchPair.AverageSSD;
    
    std::stringstream ssSSD;
    ssSSD << ssd;
    this->lblAverageSSD->setText(ssSSD.str().c_str());
    
    std::stringstream ssHistogramDifference;
    ssHistogramDifference << patchPair.HistogramDifference;
    this->lblHistogramDistance->setText(ssHistogramDifference.str().c_str());
    */
    
    Refresh();
    LeaveFunction("DisplayUsedPatchInformation()");
    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DisplayUsedPatchInformation!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::OutputPairs(const std::vector<PatchPair>& patchPairs, const std::string& filename)
{
  std::ofstream fout(filename.c_str());
  
  for(unsigned int i = 0; i < patchPairs.size(); ++i)
    {
    fout << "Potential patch " << i << ": " << std::endl
	<< "target index: " << patchPairs[i].TargetPatch.Region.GetIndex() << std::endl;
	//<< "ssd score: " << patchPairs[i].GetAverageSSD() << std::endl;
	//<< "histogram score: " << patchPairs[i].HistogramDifference << std::endl;
    }
    
  fout.close();
}

void PatchBasedInpaintingGUI::ChangeDisplayedIteration()
{
  // This should be called only when the iteration actually changed.
  
  EnterFunction("ChangeDisplayedIteration()");

  std::stringstream ss;
  ss << this->IterationToDisplay << " out of " << this->Inpainting.GetNumberOfCompletedIterations();
  this->lblCurrentIteration->setText(ss.str().c_str());

  if(this->IterationToDisplay > 0)
    {
    DisplayUsedPatches();
    HighlightUsedPatches();
    DisplayUsedPatchInformation();
    }
  else
    {
    //this->forwardLookingTableWidget->setRowCount(0);
    //this->topPatchesTableWidget->setRowCount(0);
    this->TargetPatchScene->clear();
    this->SourcePatchScene->clear();
    this->ResultPatchScene->clear();
    }

  Refresh();
  LeaveFunction("ChangeDisplayedIteration()");
}

void PatchBasedInpaintingGUI::SetupInitialIntermediateImages()
{
  EnterFunction("SetupInitialIntermediateImages()");

  this->IterationRecords.clear();

  InpaintingIterationRecord iterationRecord;

  Helpers::DeepCopy<FloatVectorImageType>(this->UserImage, iterationRecord.Image);
  //Helpers::DeepCopy<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), stack.Image);

  Helpers::DeepCopy<Mask>(this->UserMaskImage, iterationRecord.MaskImage);
  //Helpers::DeepCopy<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), stack.Boundary);

  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetPriorityFunction()->GetPriorityImage(), iterationRecord.Priority);

  this->IterationRecords.push_back(iterationRecord);

  this->qvtkWidget->GetRenderWindow()->Render();
  if(this->IterationRecords.size() != 1)
    {
    std::cerr << "this->IterationRecords.size() != 1" << std::endl;
    exit(-1);
    }
  LeaveFunction("SetupInitialIntermediateImages()");
}

void PatchBasedInpaintingGUI::IterationComplete(const PatchPair& usedPatchPair)
{
  EnterFunction("IterationComplete()");

  InpaintingIterationRecord iterationRecord;
  //HelpersOutput::WriteImage<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), "CurrentOutput.mha");
  Helpers::DeepCopy<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), iterationRecord.Image);
  Helpers::DeepCopy<Mask>(this->Inpainting.GetMaskImage(), iterationRecord.MaskImage);
  if(!this->chkOnlySaveImage->isChecked())
    {
    //Helpers::DeepCopy<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), iterationRecord.Boundary);
    Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetPriorityFunction()->GetPriorityImage(), iterationRecord.Priority);
    }

  if(this->chkRecordSteps->isChecked())
    {
    // Chop to the desired length
    for(unsigned int i = 0; i < this->Inpainting.GetPotentialCandidatePairsReference().size(); ++i)
      {
      unsigned int numberToKeep = std::min(this->Inpainting.GetPotentialCandidatePairsReference()[i].size(), this->NumberOfTopPatchesToSave);
      //std::cout << "numberToKeep: " << numberToKeep << std::endl;
      this->Inpainting.GetPotentialCandidatePairsReference()[i].erase(this->Inpainting.GetPotentialCandidatePairsReference()[i].begin() + numberToKeep,
                                                                      this->Inpainting.GetPotentialCandidatePairsReference()[i].end());
      }

    iterationRecord.PotentialPairSets = this->Inpainting.GetPotentialCandidatePairs();
    //std::cout << "iterationRecord.PotentialPairSets: " << iterationRecord.PotentialPairSets.size() << std::endl;
    }

  iterationRecord.UsedPatchPair = usedPatchPair;
  this->IterationRecords.push_back(iterationRecord);
  //std::cout << "There are now " << this->IterationRecords.size() << " recorded iterations." << std::endl;

  // After one iteration, GetNumberOfCompletedIterations will be 1. This is exactly the set of intermediate images we want to display,
  // because the 0th intermediate images are the original inputs.
  if(this->chkLive->isChecked())
    {
    this->IterationToDisplay = this->Inpainting.GetNumberOfCompletedIterations();
    //std::cout << "Switch to display iteration " << this->IterationToDisplay << std::endl;
    ChangeDisplayedIteration();

    Refresh();
    }
  else
    {
    std::stringstream ss;
    ss << this->IterationToDisplay << " out of " << this->Inpainting.GetNumberOfCompletedIterations();
    this->lblCurrentIteration->setText(ss.str().c_str());
    }
  
  LeaveFunction("Leave IterationComplete()");
}

void PatchBasedInpaintingGUI::SetupForwardLookingTable()
{
  EnterFunction("SetupForwardLookingTable()");
  if(this->IterationToDisplay < 1)
    {
    std::cout << "Can only display result patch for iterations > 0." << std::endl;
    this->ForwardLookModel->SetIterationToDisplay(0);
    this->ForwardLookModel->Refresh();
    return;
    }

  this->ForwardLookModel->SetIterationToDisplay(this->IterationToDisplay);
  this->ForwardLookModel->SetPatchDisplaySize(this->PatchDisplaySize);
  this->ForwardLookModel->Refresh();

  this->SourcePatchToDisplay = 0;
  HighlightSelectedForwardLookPatch();

  this->ForwardLookTableView->setColumnWidth(0, this->PatchDisplaySize);
  this->ForwardLookTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  this->ForwardLookTableView->verticalHeader()->setDefaultSectionSize(this->PatchDisplaySize);
  LeaveFunction("SetupForwardLookingTable()");
}

void PatchBasedInpaintingGUI::ChangeDisplayedTopPatch()
{
  EnterFunction("ChangeDisplayedTopPatch()");
  DisplaySourcePatch();
  DisplayResultPatch();

  HighlightSourcePatches();
  HighlightSelectedSourcePatch();
  LeaveFunction("ChangeDisplayedTopPatch()");
}

void PatchBasedInpaintingGUI::ChangeDisplayedForwardLookPatch()
{
  DebugMessage("ChangeDisplayedForwardLookPatch()");
  // Setup the top source patches table for this forward look patch.
  //SetupTopPatchesTable();

  // Display the big target patch
  DisplayTargetPatch();
  DisplayResultPatch();

  HighlightSelectedForwardLookPatch();
  HighlightSourcePatches();
}

void PatchBasedInpaintingGUI::SetupTopPatchesTable()
{
  DebugMessage("SetupTopPatchesTable()");
  
  if(this->IterationToDisplay < 1)
    {
    std::cout << "Can only display result patch for iterations > 0." << std::endl;
    this->TopPatchesModel->SetIterationToDisplay(0);
    this->TopPatchesModel->Refresh();
    return;
    }

  this->TopPatchesModel->SetIterationToDisplay(this->IterationToDisplay);
  this->TopPatchesModel->SetForwardLookToDisplay(this->ForwardLookToDisplay);
  this->TopPatchesModel->SetPatchDisplaySize(this->PatchDisplaySize);
  this->TopPatchesModel->SetNumberOfTopPatchesToDisplay(this->NumberOfTopPatchesToDisplay);
  this->TopPatchesModel->Refresh();

  this->SourcePatchToDisplay = 0;
  HighlightSelectedSourcePatch();
  
  //this->topPatchesTableWidget->horizontalHeader()->resizeColumnsToContents();
  //this->TopPatchesTableView->resizeColumnsToContents();
  //this->TopPatchesTableView->resizeRowsToContents();
  
  this->TopPatchesTableView->setColumnWidth(0, this->PatchDisplaySize);
  this->TopPatchesTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  this->TopPatchesTableView->verticalHeader()->setDefaultSectionSize(this->PatchDisplaySize);

  
}

void PatchBasedInpaintingGUI::HighlightSelectedForwardLookPatch()
{
  DebugMessage("HighlightSelectedForwardLookPatch()");
  // Highlight the selected forward look patch in a different color than the rest if the user has chosen to display forward look patch locations.
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display result patch for iterations > 0." << std::endl;
    return;
    }

  if(!this->chkDisplayForwardLookPatchLocations->isChecked())
    {
    return;
    }
  else
    {
    unsigned int patchRadius = this->PatchRadius;
    unsigned int patchSize = Helpers::SideLengthFromRadius(patchRadius);

    this->SelectedForwardLookOutlineLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
    this->SelectedForwardLookOutlineLayer.ImageData->AllocateScalars();
    unsigned char selectedPatchColor[3];
    HelpersQt::QColorToUCharColor(this->SelectedForwardLookPatchColor, selectedPatchColor);
    Helpers::BlankAndOutlineImage(this->SelectedForwardLookOutlineLayer.ImageData, selectedPatchColor);
    unsigned char centerPixelColor[3];
    HelpersQt::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);
    Helpers::SetImageCenterPixel(this->SelectedForwardLookOutlineLayer.ImageData, centerPixelColor);

    // There is a -1 offset here because the 0th patch pair to be stored is after iteration 1
    // (as after the 0th iteration (initial conditions) there are no used patch pairs)
    std::vector<CandidatePairs>& allCandidatePairs = this->IterationRecords[this->IterationToDisplay].PotentialPairSets;

    const Patch& selectedPatch = allCandidatePairs[this->ForwardLookToDisplay].TargetPatch;
    this->SelectedForwardLookOutlineLayer.ImageSlice->SetPosition(selectedPatch.Region.GetIndex()[0], selectedPatch.Region.GetIndex()[1], 0);

    this->qvtkWidget->GetRenderWindow()->Render();
    }

}


void PatchBasedInpaintingGUI::HighlightSelectedSourcePatch()
{
  DebugMessage("HighlightSelectedSourcePatch()");
  // Highlight the selected source patch in a different color than the rest if the user has chosen to display forward look patch locations.
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display result patch for iterations > 0." << std::endl;
    return;
    }

  if(!this->chkDisplaySourcePatchLocations->isChecked())
    {
    return;
    }
  else
    {
    unsigned int patchRadius = this->PatchRadius;
    unsigned int patchSize = Helpers::SideLengthFromRadius(patchRadius);

    this->SelectedSourcePatchOutlineLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
    this->SelectedSourcePatchOutlineLayer.ImageData->AllocateScalars();
    unsigned char selectedPatchColor[3];
    HelpersQt::QColorToUCharColor(this->SelectedSourcePatchColor, selectedPatchColor);
    Helpers::BlankAndOutlineImage(this->SelectedSourcePatchOutlineLayer.ImageData, selectedPatchColor);
    
    unsigned char centerPixelColor[3];
    HelpersQt::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);
    Helpers::SetImageCenterPixel(this->SelectedSourcePatchOutlineLayer.ImageData, centerPixelColor);

    // There is a -1 offset here because the 0th patch pair to be stored is after iteration 1
    // (as after the 0th iteration (initial conditions) there are no used patch pairs)
    const CandidatePairs& candidatePairs = this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay];

    const Patch& selectedPatch = candidatePairs[this->SourcePatchToDisplay].SourcePatch;
    this->SelectedSourcePatchOutlineLayer.ImageSlice->SetPosition(selectedPatch.Region.GetIndex()[0], selectedPatch.Region.GetIndex()[1], 0);

    this->qvtkWidget->GetRenderWindow()->Render();
    }
}

void PatchBasedInpaintingGUI::InitializeGUIElements()
{
  on_chkLive_clicked();

  this->PatchRadius = this->txtPatchRadius->text().toUInt();

  this->NumberOfTopPatchesToSave = this->txtNumberOfTopPatchesToSave->text().toUInt();

  this->NumberOfForwardLook = this->txtNumberOfForwardLook->text().toUInt();

  this->GoToIteration = this->txtGoToIteration->text().toUInt();

  this->NumberOfTopPatchesToDisplay = this->txtNumberOfTopPatchesToDisplay->text().toUInt();

  this->UserPatchLayer.ImageSlice->SetVisibility(this->chkDisplayUserPatch->isChecked());
}


void PatchBasedInpaintingGUI::SetComparisonFunctionsFromGUI()
{
  this->DifferenceFunctionsToCompute.clear();
  if(this->chkCompareFull->isChecked())
    {
    this->PatchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,this->PatchCompare,_1));
    }
  if(this->chkCompareColor->isChecked())
    {
    this->PatchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchColorDifference,this->PatchCompare,_1));
    }
  if(this->chkCompareDepth->isChecked())
    {
    this->PatchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchDepthDifference,this->PatchCompare,_1));
    }
}

void PatchBasedInpaintingGUI::SetSortFunctionFromGUI()
{
  if(this->radSortByFullDifference->isChecked())
    {
    this->Inpainting.PatchSortFunction = &SortByAverageSquaredDifference;
    }
  else if(this->radSortByColorDifference->isChecked())
    {
    this->Inpainting.PatchSortFunction = &SortByColorDifference;
    }
  else if(this->radSortByDepthDifference->isChecked())
    {
    this->Inpainting.PatchSortFunction = &SortByDepthDifference;
    }
  else if(this->radSortByColorAndDepth->isChecked())
    {
    this->Inpainting.PatchSortFunction = &SortByDepthAndColor;
    }
}

void PatchBasedInpaintingGUI::SetDepthColorLambdaFromGUI()
{
  PatchPair::DepthColorLambda = static_cast<float>(sldDepthColorLambda->value())/100.0f;
  std::cout << "DepthColorLambda set to " << PatchPair::DepthColorLambda << std::endl;
}
