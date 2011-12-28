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
#include <QGraphicsPixmapItem>
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
#include "ModelView/ListModelDisplay.h"
#include "ModelView/ListModelSave.h"
#include "Mask.h"
#include "PatchSorting.h"
#include "PixmapDelegate.h"
#include "PriorityCriminisi.h"
#include "PriorityDepth.h"
#include "PriorityManual.h"
#include "PriorityOnionPeel.h"
#include "PriorityRandom.h"
#include "SelfPatchCompare.h"
#include "Types.h"

Q_DECLARE_METATYPE(PatchPair)

void PatchBasedInpaintingGUI::DefaultConstructor()
{
  // This function is called by both constructors. This avoid code duplication.
  EnterFunction("PatchBasedInpaintingGUI::DefaultConstructor()");

  this->Inpainting = NULL;
  this->RecordToDisplay = NULL;

  this->setupUi(this);

  SetDefaultValues();
  SetupCamera();

  SetupColors();

  SetupScenes();

  SetupToolbar();

  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();
  this->InteractorStyle->TrackballStyle->AddObserver(CustomTrackballStyle::PatchesMovedEvent, this, &PatchBasedInpaintingGUI::UserPatchMoved);

  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  SetupLayers();
  
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  this->UserImage = FloatVectorImageType::New();
  this->UserMaskImage = Mask::New();

  SetupComputationThread();

  SetupConnections();

  SetProgressBarToMarquee();

  InitializeGUIElements();

  ConnectForwardLookModelToView();
  ConnectTopPatchesModelToView();
  
  SetupUserPatch();

  SetupValidators();

  SetupImageModels();

  LeaveFunction("PatchBasedInpaintingGUI::DefaultConstructor()");
}

void PatchBasedInpaintingGUI::SetDefaultValues()
{
  this->PatchRadius = 10;
  this->NumberOfTopPatchesToSave = 0;
  this->NumberOfForwardLook = 0;
  this->GoToIteration = 0;
  this->NumberOfTopPatchesToDisplay = 0;

  this->PatchDisplaySize = 100;

  this->IterationToDisplay = 0;
  this->ForwardLookToDisplayId = 0;
  this->SourcePatchToDisplayId = 0;
}

void PatchBasedInpaintingGUI::SetupImageModels()
{
  this->ModelSave = new ListModelSave;
  this->listViewSave->setModel(this->ModelSave);

  this->ModelDisplay = new ListModelDisplay;
  this->listViewDisplay->setModel(this->ModelDisplay);
  connect (this->listViewDisplay, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_ChangeDisplayedImages(QModelIndex)));

  this->ModelImages = new TableModelImageInput;
  this->ModelImages->setItems(&this->ImageInputs);
  this->tableViewImages->setModel(this->ModelImages);
  connect (this->tableViewImages, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_ChangeFileName(QModelIndex)));
}

void PatchBasedInpaintingGUI::SetupComputationThread()
{
  this->ComputationThread = new QThread;
  this->InpaintingComputation = new InpaintingComputationObject;
  this->InpaintingComputation->moveToThread(this->ComputationThread);
}

void PatchBasedInpaintingGUI::SetupToolbar()
{
  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");
  QIcon saveIcon = QIcon::fromTheme("document-save");

  // Attach icons to actions
  actionOpen->setIcon(openIcon);
  this->toolBar->addAction(actionOpen);

  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);
}

void PatchBasedInpaintingGUI::SetupUserPatch()
{
  Helpers::CreateTransparentVTKImage(Helpers::SizeFromRadius(this->PatchRadius), this->UserPatchLayer.ImageData);
  unsigned char userPatchColor[3];
  HelpersQt::QColorToUCharColor(this->UserPatchColor, userPatchColor);
  Helpers::BlankAndOutlineImage(this->UserPatchLayer.ImageData, userPatchColor);

  itk::Size<2> patchSize = Helpers::SizeFromRadius(this->PatchRadius);
  this->UserPatchRegion.SetSize(patchSize);
}

void PatchBasedInpaintingGUI::SetupLayers()
{
  this->UserPatchLayer.ImageSlice->SetPickable(true);

  this->ImageLayer.ImageSlice->SetPickable(false);
  this->MaskLayer.ImageSlice->SetPickable(false);
  this->UsedTargetPatchLayer.ImageSlice->SetPickable(false);
  this->UsedSourcePatchLayer.ImageSlice->SetPickable(false);
  this->AllSourcePatchOutlinesLayer.ImageSlice->SetPickable(false);
  this->AllForwardLookOutlinesLayer.ImageSlice->SetPickable(false);

  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);
  this->Renderer->AddViewProp(this->MaskLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedTargetPatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedSourcePatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->AllSourcePatchOutlinesLayer.ImageSlice);
  this->Renderer->AddViewProp(this->AllForwardLookOutlinesLayer.ImageSlice);

  this->Renderer->AddViewProp(this->UserPatchLayer.ImageSlice);
}

void PatchBasedInpaintingGUI::SetupValidators()
{
  this->IntValidator = new QIntValidator(0, 10000, this);
  this->txtPatchRadius->setValidator(this->IntValidator);
  this->txtNumberOfTopPatchesToSave->setValidator(this->IntValidator);
  this->txtNumberOfForwardLook->setValidator(this->IntValidator);
  this->txtGoToIteration->setValidator(this->IntValidator);
  this->txtNumberOfTopPatchesToDisplay->setValidator(this->IntValidator);
}

void PatchBasedInpaintingGUI::SetProgressBarToMarquee()
{
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();
}

void PatchBasedInpaintingGUI::ConnectForwardLookModelToView()
{
  this->ForwardLookModel = new TableModelForwardLook(this->IterationRecords, this->ImageDisplayStyle);
  this->ForwardLookTableView->setModel(this->ForwardLookModel);
  this->ForwardLookTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  PixmapDelegate* forwardLookPixmapDelegate = new PixmapDelegate;
  this->ForwardLookTableView->setItemDelegateForColumn(0, forwardLookPixmapDelegate);

  this->connect(this->ForwardLookTableView->selectionModel(), SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )),
                SLOT(slot_ForwardLookTableView_changed(const QModelIndex & , const QModelIndex & )));
}

void PatchBasedInpaintingGUI::ConnectTopPatchesModelToView()
{
  this->TopPatchesModel = new TableModelTopPatches(this->IterationRecords, this->ImageDisplayStyle);
  this->TopPatchesTableView->setModel(this->TopPatchesModel);
  this->TopPatchesTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  PixmapDelegate* topPatchesPixmapDelegate = new PixmapDelegate;
  this->TopPatchesTableView->setItemDelegateForColumn(0, topPatchesPixmapDelegate);

  this->connect(this->TopPatchesTableView->selectionModel(), SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )),
                SLOT(slot_TopPatchesTableView_changed(const QModelIndex & , const QModelIndex & )));
}

void PatchBasedInpaintingGUI::SetupScenes()
{
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
}

void PatchBasedInpaintingGUI::SetupCamera()
{
  this->CameraLeftToRightVector.resize(3);
  this->CameraLeftToRightVector[0] = -1;
  this->CameraLeftToRightVector[1] = 0;
  this->CameraLeftToRightVector[2] = 0;

  this->CameraBottomToTopVector.resize(3);
  this->CameraBottomToTopVector[0] = 0;
  this->CameraBottomToTopVector[1] = 1;
  this->CameraBottomToTopVector[2] = 0;
}

void PatchBasedInpaintingGUI::SetupConnections()
{
  qRegisterMetaType<PatchPair>("PatchPair");
  connect(this->ComputationThread, SIGNAL(started()), this->InpaintingComputation, SLOT(start()));
  connect(this->InpaintingComputation, SIGNAL(InpaintingComplete()), this->ComputationThread, SLOT(quit()));
  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair&)), this->ComputationThread, SLOT(quit()));

  connect(this->ComputationThread, SIGNAL(started()), this, SLOT(slot_StartProgress()), Qt::QueuedConnection);
  connect(this->InpaintingComputation, SIGNAL(InpaintingComplete()), this, SLOT(slot_StopProgress()), Qt::QueuedConnection);
  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair&)), this, SLOT(slot_StopProgress()), Qt::QueuedConnection);

  // Using a blocking connection allows everything (computation and drawing) to be performed sequentially which is helpful for debugging,
  // but makes the interface very very choppy.
  // We are assuming that the computation takes longer than the drawing.
  //connect(&ComputationThread, SIGNAL(IterationCompleteSignal()), this, SLOT(IterationCompleteSlot()), Qt::QueuedConnection);

  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair&)),
          this, SLOT(slot_IterationComplete(const PatchPair&)), Qt::BlockingQueuedConnection);

}

void PatchBasedInpaintingGUI::showEvent ( QShowEvent * event )
{
  SetupSaveModel();
}

// Default constructor
PatchBasedInpaintingGUI::PatchBasedInpaintingGUI()
{
  DefaultConstructor();
};

PatchBasedInpaintingGUI::PatchBasedInpaintingGUI(const std::string& imageFileName, const std::string& maskFileName, const bool debugEnterLeave = false)
{
  this->SetDebugFunctionEnterLeave(debugEnterLeave);

  EnterFunction("PatchBasedInpaintingGUI(string, string, bool)");

  std::cout << "Image: " << imageFileName << " Mask: " << maskFileName << std::endl;

  DefaultConstructor();

  OpenImage(imageFileName);
  OpenMask(maskFileName, false);

  
  Initialize();
  LeaveFunction("PatchBasedInpaintingGUI(string, string, bool)");
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
  patchCompare->SetImage(dynamic_cast<FloatVectorImageType*>(this->IterationRecords[iterationToCompare].GetImageByName("Image").Image.GetPointer()));
  patchCompare->SetMask(dynamic_cast<Mask*>(this->IterationRecords[iterationToCompare].GetImageByName("Mask").Image.GetPointer()));
  patchCompare->SetNumberOfComponentsPerPixel(this->UserImage->GetNumberOfComponentsPerPixel());
  patchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,patchCompare,_1));
  CandidatePairs candidatePairs(this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplayId].TargetPatch);
  Patch userPatch(this->UserPatchRegion);
  candidatePairs.AddPairFromPatch(userPatch);
  patchCompare->SetPairs(&candidatePairs);
  patchCompare->ComputeAllSourceDifferences();

  std::stringstream ss;
  ss << candidatePairs[0].DifferenceMap[PatchPair::AverageAbsoluteDifference];
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
  this->UserPatchColor = Qt::yellow;
  //this->HoleColor = Qt::gray;
  this->HoleColor.setRgb(255, 153, 0); // Orange
  this->SceneBackgroundColor.setRgb(153, 255, 0); // Lime green
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

  this->ImageInputs.push_back(ImageInput("Mask", fileName.c_str()));

  UpdateAllImageInputModels();
  //Helpers::DebugWriteImageConditional<Mask>(this->UserMaskImage, "Debug/InvertedMask.png", this->DebugImages);
}

void PatchBasedInpaintingGUI::UpdateAllImageInputModels()
{
  this->ModelSave->Refresh();
  //this->ModelDisplay->Refresh();
  this->ModelImages->Refresh();
}

void PatchBasedInpaintingGUI::OpenImage(const std::string& fileName)
{
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

  this->ImageInputs.push_back(ImageInput("Image", fileName.c_str()));

  UpdateAllImageInputModels();
}

void PatchBasedInpaintingGUI::Reset()
{
  this->tabSettings->setEnabled(true);
  this->tabRecord->setEnabled(true);
  this->tabImages->setEnabled(true);

  this->btnReset->setEnabled(false);
  this->btnInpaint->setEnabled(false);
  this->btnStep->setEnabled(false);
  this->btnInitialize->setEnabled(true);

  this->IterationRecords.clear();
  Initialize();
  Refresh();
}

void PatchBasedInpaintingGUI::DisplayMask()
{
  //vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  //Helpers::ITKScalarImageToScaledVTKImage<Mask>(this->IntermediateImages[this->IterationToDisplay].MaskImage, temp);
  //Helpers::MakeValidPixelsTransparent(temp, this->MaskLayer.ImageData, 0); // Set the zero pixels of the mask to transparent

  dynamic_cast<Mask*>(this->IterationRecords[this->IterationToDisplay].GetImageByName("Mask").Image.GetPointer())->MakeVTKImage(this->MaskLayer.ImageData, QColor(Qt::white), this->HoleColor, false, true); // (..., holeTransparent, validTransparent);
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
  QImage userPatch = HelpersQt::GetQImage<FloatVectorImageType>(dynamic_cast<FloatVectorImageType*>(this->IterationRecords[this->IterationToDisplay].GetImageByName("Image").Image.GetPointer()),
                                                                this->UserPatchRegion, this->ImageDisplayStyle);
  //userPatch = HelpersQt::FitToGraphicsView(userPatch, gfxTarget);
  QGraphicsPixmapItem* item = this->UserPatchScene->addPixmap(QPixmap::fromImage(userPatch));
  gfxTarget->fitInView(item);
  LeaveFunction("DisplayUserPatch");
}

void PatchBasedInpaintingGUI::DisplayImage()
{
  EnterFunction("DisplayImage");
  HelpersDisplay::ITKVectorImageToVTKImage(dynamic_cast<FloatVectorImageType*>(this->IterationRecords[this->IterationToDisplay].GetImageByName("Image").Image.GetPointer()), this->ImageLayer.ImageData, this->ImageDisplayStyle);

  this->qvtkWidget->GetRenderWindow()->Render();
  LeaveFunction("DisplayImage");
}

// void PatchBasedInpaintingGUI::DisplayBoundary()
// {
//   EnterFunction("DisplayBoundary");
//   Helpers::ITKScalarImageToScaledVTKImage<UnsignedCharScalarImageType>(dynamic_cast<UnsignedCharScalarImageType*>(this->IterationRecords[this->IterationToDisplay].Images.FindImageByName("Boundary").Image.GetPointer()), this->BoundaryLayer.ImageData);
//   this->qvtkWidget->GetRenderWindow()->Render();
//   LeaveFunction("DisplayBoundary");
// }

// void PatchBasedInpaintingGUI::DisplayPriorityImages()
// {
//   EnterFunction("DisplayPriorityImages");
// 
//   for(unsigned int i = 0; i < this->PriorityImageCheckBoxes.size(); ++i)
//     {
//     if(this->PriorityImageCheckBoxes[i]->isChecked())
//       {
//       std::cout << "Image name: " << this->PriorityImageCheckBoxes[i]->text().toStdString() << std::endl;
//       Layer newLayer;
//       NamedVTKImage namedImage = FindImageByName(this->Inpainting->GetPriorityFunction()->GetNamedImages(), this->PriorityImageCheckBoxes[i]->text().toStdString());
//       newLayer.ImageData = namedImage.ImageData;
//       newLayer.Setup();
//       newLayer.ImageSlice->SetPickable(false);
// 
//       this->Renderer->AddViewProp(newLayer.ImageSlice);
//       }
//     }
// 
//   this->qvtkWidget->GetRenderWindow()->Render();
//   LeaveFunction("DisplayPriorityImages");
// }

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

//     if(this->chkDisplayImage->isChecked())
//       {
//       DisplayImage();
//       }
// 
//     if(this->chkDisplayMask->isChecked())
//       {
//       DisplayMask();
//       }
// 
//     if(this->chkDisplayBoundary->isChecked())
//       {
//       DisplayBoundary();
//       }

    //DisplayPriorityImages();

    this->UsedSourcePatchLayer.ImageSlice->SetVisibility(this->chkHighlightUsedPatches->isChecked());
    this->UsedTargetPatchLayer.ImageSlice->SetVisibility(this->chkHighlightUsedPatches->isChecked());

    this->AllForwardLookOutlinesLayer.ImageSlice->SetVisibility(this->chkDisplayForwardLookPatchLocations->isChecked());
    if(this->chkDisplayForwardLookPatchLocations->isChecked())
      {
      HighlightForwardLookPatches();
      }

    this->AllSourcePatchOutlinesLayer.ImageSlice->SetVisibility(this->chkDisplaySourcePatchLocations->isChecked());
    if(this->chkDisplaySourcePatchLocations->isChecked())
      {
      HighlightSourcePatches();
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

//   if(!this->txtBlurredImage->text().isEmpty())
//     {
//     typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
//     ReaderType::Pointer reader = ReaderType::New();
//     reader->SetFileName(this->txtBlurredImage->text().toStdString());
//     reader->Update();
//     //this->Inpainting->SetBlurredImage(reader->GetOutput());
//     }
// 
//   if(!this->txtMembershipImage->text().isEmpty())
//     {
//     typedef itk::ImageFileReader<IntImageType> ReaderType;
//     ReaderType::Pointer reader = ReaderType::New();
//     reader->SetFileName(this->txtMembershipImage->text().toStdString());
//     reader->Update();
//     //this->Inpainting->SetMembershipImage(reader->GetOutput());
//     }

  // The PatchSortFunction has already been set by the radio buttons.

  std::cout << "User Image size: " << this->UserImage->GetLargestPossibleRegion().GetSize() << std::endl;
  std::cout << "User Mask size: " << this->UserMaskImage->GetLargestPossibleRegion().GetSize() << std::endl;
  HelpersOutput::WriteImage<Mask>(this->UserMaskImage, "mask.mha");

//   CreateInitialRecord();
//   this->IterationToDisplay = 0;
//   ChangeDisplayedIteration();
// 
//   SetCheckboxVisibility(true);

  LeaveFunction("PatchBasedInpaintingGUI::Initialize()");
}

void PatchBasedInpaintingGUI::DisplaySourcePatch()
{
  try
  {
    EnterFunction("DisplaySourcePatch()");

    if(!this->RecordToDisplay)
      {
      LeaveFunction("DisplaySourcePatch()");
      return;
      }

    FloatVectorImageType::Pointer currentImage = dynamic_cast<FloatVectorImageType*>(this->RecordToDisplay->GetImageByName("Image").Image.GetPointer());

    QImage sourceImage = HelpersQt::GetQImage<FloatVectorImageType>(currentImage, this->SourcePatchToDisplay.Region, this->ImageDisplayStyle);
    //sourceImage = HelpersQt::FitToGraphicsView(sourceImage, gfxSource);
    QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));
    gfxSource->fitInView(item);
    LeaveFunction("DisplaySourcePatch()");
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
  // We use the previous image and previous mask, but the current PotentialPairSets, as these are the sets that were used to get to this state.
  try
  {
    EnterFunction("DisplayTargetPatch()");

    // The last iteration record will not have any potential patches, because there is nothing left to inpaint!
    if(!RecordToDisplay)
      {
      LeaveFunction("DisplayTargetPatch()");
      return;
      }
    FloatVectorImageType::Pointer currentImage = dynamic_cast<FloatVectorImageType*>(this->RecordToDisplay->GetImageByName("Image").Image.GetPointer());

    // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration
    // (as the current mask has been cleared where the target patch was copied).
    //Mask::Pointer currentMask = dynamic_cast<Mask*>(this->RecordToDisplay->Images.FindImageByName("Mask").Image.GetPointer());

    // Target
    QImage targetImage = HelpersQt::GetQImage<FloatVectorImageType>(currentImage, this->TargetPatchToDisplay.Region, this->ImageDisplayStyle);

    //targetImage = HelpersQt::FitToGraphicsView(targetImage, gfxTarget);
    QGraphicsPixmapItem* item = this->TargetPatchScene->addPixmap(QPixmap::fromImage(targetImage));
    gfxTarget->fitInView(item);
    LeaveFunction("DisplayTargetPatch()");
    }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in DisplayTargetPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void PatchBasedInpaintingGUI::DisplayResultPatch()
{
  EnterFunction("DisplayResultPatch()");
  try
  {
    if(!RecordToDisplay)
      {
      LeaveFunction("DisplayResultPatch()");
      return;
      }

    FloatVectorImageType::Pointer currentImage = dynamic_cast<FloatVectorImageType*>(this->RecordToDisplay->GetImageByName("Image").Image.GetPointer());

    // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration
    // (as the current mask has been cleared where the target patch was copied).
    Mask::Pointer currentMask = dynamic_cast<Mask*>(this->RecordToDisplay->GetImageByName("Mask").Image.GetPointer());

    itk::Size<2> regionSize = this->Inpainting->GetPatchSize();

    QImage qimage(regionSize[0], regionSize[1], QImage::Format_RGB888);

    itk::ImageRegionIterator<FloatVectorImageType> sourceIterator(currentImage, this->SourcePatchToDisplay.Region);
    itk::ImageRegionIterator<FloatVectorImageType> targetIterator(currentImage, this->TargetPatchToDisplay.Region);
    itk::ImageRegionIterator<Mask> maskIterator(currentMask, this->TargetPatchToDisplay.Region);

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

      itk::Offset<2> offset = sourceIterator.GetIndex() - this->SourcePatchToDisplay.Region.GetIndex();
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

    //qimage = HelpersQt::FitToGraphicsView(qimage, gfxResult);
    this->ResultPatchScene->clear();
    QGraphicsPixmapItem* item = this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));
    gfxResult->fitInView(item);
    //this->ResultPatchScene->addPixmap(QPixmap());
    //std::cout << "Set result patch." << std::endl;
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

    if(!RecordToDisplay)
      {
      return;
      }
    // If the user has not requested to display the patches, quit.
    if(!this->chkDisplayForwardLookPatchLocations->isChecked())
      {
      DebugMessage("HighlightForwardLookPatches: chkDisplayForwardLookPatchLocations not checked!");
      return;
      }

    // Get the candidate patches and make sure we have requested a valid set.
    const std::vector<CandidatePairs>& candidatePairs = this->RecordToDisplay->PotentialPairSets;

    unsigned char centerPixelColor[3];
    HelpersQt::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);

    for(unsigned int candidateId = 0; candidateId < candidatePairs.size(); ++candidateId)
      {
      unsigned char borderColor[3];
      if(candidateId == this->ForwardLookToDisplayId)
        {
        HelpersQt::QColorToUCharColor(this->SelectedForwardLookPatchColor, borderColor);
        }
      else
        {
        HelpersQt::QColorToUCharColor(this->AllForwardLookPatchColor, borderColor);
        }

      const Patch& currentPatch = candidatePairs[candidateId].TargetPatch;
      //std::cout << "Outlining " << currentPatch.Region << std::endl;
      //DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);

      Helpers::BlankAndOutlineRegion(this->AllForwardLookOutlinesLayer.ImageData, currentPatch.Region, borderColor);

      Helpers::SetRegionCenterPixel(this->AllForwardLookOutlinesLayer.ImageData, currentPatch.Region, centerPixelColor);
      }

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

    // Delete any current highlight patches. We want to delete these (if they exist) no matter what because then
    // they won't be displayed if the box is not checked (they will respect the check box).
    Helpers::BlankImage(this->AllSourcePatchOutlinesLayer.ImageData);

    if(!this->RecordToDisplay)
      {
      DebugMessage("HighlightSourcePatches: !this->RecordToDisplay");
      LeaveFunction("HighlightSourcePatches()");
      return;
      }

    if(!this->chkDisplaySourcePatchLocations->isChecked())
      {
      DebugMessage("HighlightSourcePatches: !this->chkDisplaySourcePatchLocations->isChecked()");
      LeaveFunction("HighlightSourcePatches()");
      return;
      }

    unsigned char centerPixelColor[3];
    HelpersQt::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);

    const CandidatePairs& candidatePairs = this->RecordToDisplay->PotentialPairSets[this->ForwardLookToDisplayId];
    unsigned int numberToDisplay = std::min(candidatePairs.size(), NumberOfTopPatchesToDisplay);
    DebugMessage<unsigned int>("HighlightSourcePatches: Displaying patches: ", numberToDisplay);
    unsigned char borderColor[3];

    for(unsigned int candidateId = 0; candidateId < numberToDisplay; ++candidateId)
      {
      if(candidateId == this->SourcePatchToDisplayId)
        {
        HelpersQt::QColorToUCharColor(this->SelectedSourcePatchColor, borderColor);
        }
      else
        {
        HelpersQt::QColorToUCharColor(this->AllSourcePatchColor, borderColor);
        }

      const Patch& currentPatch = candidatePairs[candidateId].SourcePatch;
      //DebugMessage<itk::ImageRegion<2> >("HighlightSourcePatches: Display patch: ", currentPatch.Region);
      Helpers::BlankAndOutlineRegion(this->AllSourcePatchOutlinesLayer.ImageData, currentPatch.Region, borderColor);
      Helpers::SetRegionCenterPixel(this->AllSourcePatchOutlinesLayer.ImageData, currentPatch.Region, centerPixelColor);
      }

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
  EnterFunction("HighlightUsedPatches()");
  try
  {
//     // The last iteration record will not have any potential patches, because there is nothing left to inpaint!
//     if(!this->RecordToDisplay->PotentialPairSets.size())
//       {
//       LeaveFunction("HighlightUsedPatches()");
//       return;
//       }
//
//     PatchPair patchPair = this->RecordToDisplay->UsedPatchPair;
//
//     unsigned char centerPixelColor[3];
//     HelpersQt::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);
//
//     // Target
//     DebugMessage("Target...");
//     Patch targetPatch = patchPair.TargetPatch;
//
//     unsigned int patchSize = targetPatch.Region.GetSize()[0];
//     //std::cout << "Displaying used target patch " << this->CurrentUsedPatchDisplayed << " : " << targetPatch.Region << std::endl;
//     DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);
//     this->UsedTargetPatchLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
//     unsigned char targetPatchColor[3];
//     HelpersQt::QColorToUCharColor(this->UsedTargetPatchColor, targetPatchColor);
//     Helpers::BlankAndOutlineImage(this->UsedTargetPatchLayer.ImageData, targetPatchColor);
//     Helpers::SetImageCenterPixel(this->UsedTargetPatchLayer.ImageData, centerPixelColor);
//     this->UsedTargetPatchLayer.ImageSlice->SetPosition(targetPatch.Region.GetIndex()[0], targetPatch.Region.GetIndex()[1], 0);
//
//     // Source
//     DebugMessage("Source...");
//     Patch sourcePatch = patchPair.SourcePatch;
//
//     //std::cout << "Displaying used source patch " << this->CurrentUsedPatchDisplayed << " : " << sourcePatch.Region << std::endl;
//     DebugMessage<itk::ImageRegion<2> >("Source patch region: ", sourcePatch.Region);
//     this->UsedSourcePatchLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
//     unsigned char sourcePatchColor[3];
//     HelpersQt::QColorToUCharColor(this->UsedSourcePatchColor, sourcePatchColor);
//     Helpers::BlankAndOutlineImage(this->UsedSourcePatchLayer.ImageData, sourcePatchColor);
//     Helpers::SetImageCenterPixel(this->UsedSourcePatchLayer.ImageData, centerPixelColor);
//     this->UsedSourcePatchLayer.ImageSlice->SetPosition(sourcePatch.Region.GetIndex()[0], sourcePatch.Region.GetIndex()[1], 0);
//
//     Refresh();
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
      //std::cerr << "Can only display used patch information for iterations >= 1" << std::endl;
      return;
      }

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

  if(this->IterationToDisplay > this->IterationRecords.size())
    {
    std::cout << "this->IterationToDisplay > this->IterationRecords.size()" << std::endl;
    std::cout << "this->IterationToDisplay: " << this->IterationToDisplay << std::endl;
    std::cout << "this->IterationRecords.size(): " << this->IterationRecords.size() << std::endl;
    this->RecordToDisplay = NULL;
    LeaveFunction("ChangeDisplayedIteration()");
    return;
    }

  // If there are no PotentialPairSets, we can't display them.
  if(this->IterationRecords[this->IterationToDisplay].PotentialPairSets.size() == 0)
    {
    LeaveFunction("ChangeDisplayedIteration()");
    return;
    }

  this->RecordToDisplay = &IterationRecords[this->IterationToDisplay];

  this->ModelDisplay->SetIterationRecord(this->RecordToDisplay);

  this->SourcePatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->ForwardLookToDisplayId][this->SourcePatchToDisplayId].SourcePatch;
  this->TargetPatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->ForwardLookToDisplayId].TargetPatch;

  std::stringstream ss;
  ss << this->IterationToDisplay << " out of " << this->Inpainting->GetNumberOfCompletedIterations();
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

void PatchBasedInpaintingGUI::CreateInitialRecord()
{
  EnterFunction("CreateInitialRecord()");

  this->IterationRecords.clear();

  InpaintingIterationRecord iterationRecord;

  //NamedITKImage::Pointer image = dynamic_cast<NamedITKImage*>(FloatVectorImageType::New().GetPointer());
  FloatVectorImageType::Pointer image = FloatVectorImageType::New();
  Helpers::DeepCopy<FloatVectorImageType>(this->UserImage, image);
  NamedITKImage namedImage;
  namedImage.Image = image;
  namedImage.Name = "Image";
  iterationRecord.AddImage(namedImage);

  //Helpers::DeepCopy<FloatVectorImageType>(this->UserImage, dynamic_cast<FloatVectorImageType*>(iterationRecord.Images.FindImageByName("Image")));
  //Helpers::DeepCopy<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), stack.Image);

  Mask::Pointer maskImage = Mask::New();
  Helpers::DeepCopy<Mask>(this->UserMaskImage, maskImage);
  NamedITKImage namedMaskImage;
  namedMaskImage.Name = "Mask";
  namedMaskImage.Image = maskImage;
  iterationRecord.AddImage(namedMaskImage);
  //Helpers::DeepCopy<Mask>(this->UserMaskImage, dynamic_cast<Mask*>(iterationRecord.Images.FindImageByName("Mask")));
  //Helpers::DeepCopy<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), stack.Boundary);

  FloatScalarImageType::Pointer priorityImage = FloatScalarImageType::New();
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting->GetPriorityFunction()->GetPriorityImage(), priorityImage);
  NamedITKImage namedPriorityImage;
  namedPriorityImage.Name = "Priority";
  namedPriorityImage.Image = priorityImage;
  iterationRecord.AddImage(namedPriorityImage);

  //Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetPriorityFunction()->GetPriorityImage(), dynamic_cast<FloatScalarImageType*>(iterationRecord.Images.FindImageByName("Priority")));

  this->IterationRecords.push_back(iterationRecord);

  this->qvtkWidget->GetRenderWindow()->Render();
  if(this->IterationRecords.size() != 1)
    {
    std::cerr << "this->IterationRecords.size() != 1" << std::endl;
    exit(-1);
    }
  LeaveFunction("CreateInitialRecord()");
}

void PatchBasedInpaintingGUI::IterationComplete(const PatchPair& usedPatchPair)
{
  EnterFunction("IterationComplete()");

  InpaintingIterationRecord iterationRecord;
  for(unsigned int i = 0; i < this->Inpainting->GetImagesToUpdate().size(); ++i)
    {
    std::cout << "Updating image " << i << std::endl;

    Helpers::OutputImageType(this->Inpainting->GetImagesToUpdate()[i]);
    itk::ImageBase<2>::Pointer newImage = Helpers::CreateImageWithSameType(this->Inpainting->GetImagesToUpdate()[i]);
    Helpers::OutputImageType(newImage);
    Helpers::DeepCopy(this->Inpainting->GetImagesToUpdate()[i], newImage);
    }
  std::cout << "Finished creating record images." << std::endl;

  //HelpersOutput::WriteImage<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), "CurrentOutput.mha");
  //Helpers::DeepCopy<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), dynamic_cast<FloatVectorImageType*>(iterationRecord.Images.FindImageByName("Image")));
  //Helpers::DeepCopy<Mask>(this->Inpainting.GetMaskImage(), dynamic_cast<Mask*>(iterationRecord.Images.FindImageByName("Mask")));
//   if(!this->chkOnlySaveImage->isChecked())
//     {
//     //Helpers::DeepCopy<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), iterationRecord.Boundary);
//     //Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetPriorityFunction()->GetPriorityImage(), dynamic_cast<FloatScalarImageType*>(iterationRecord.Images.FindImageByName("Priority")));
//     }

  if(this->chkRecordSteps->isChecked())
    {
    std::cout << "Recording step." << std::endl;

    // Chop to the desired length
    for(unsigned int i = 0; i < this->Inpainting->GetPotentialCandidatePairsReference().size(); ++i)
      {
      unsigned int numberToKeep = std::min(this->Inpainting->GetPotentialCandidatePairsReference()[i].size(), this->NumberOfTopPatchesToSave);
      //std::cout << "numberToKeep: " << numberToKeep << std::endl;
      this->Inpainting->GetPotentialCandidatePairsReference()[i].erase(this->Inpainting->GetPotentialCandidatePairsReference()[i].begin() + numberToKeep,
                                                                       this->Inpainting->GetPotentialCandidatePairsReference()[i].end());
      }

    // Add the patch pairs to the new record. This will mean the pairs are 1 record out of sync with the images. The interpretation would be:
    // "These are the patches that were considered to get here".
    //iterationRecord.PotentialPairSets = this->Inpainting.GetPotentialCandidatePairs();

    // Add the patch pairs to the previous record. The interpretation is "these patches are considered here, to produce the next image".
    // There should always be a previous record, because an initial record is created for the initial state.
    this->IterationRecords[this->IterationRecords.size() - 1].PotentialPairSets = this->Inpainting->GetPotentialCandidatePairs();
    //std::cout << "iterationRecord.PotentialPairSets: " << iterationRecord.PotentialPairSets.size() << std::endl;
    }

  iterationRecord.UsedPatchPair = usedPatchPair;
  this->IterationRecords.push_back(iterationRecord);
  //std::cout << "There are now " << this->IterationRecords.size() << " recorded iterations." << std::endl;

  // After one iteration, GetNumberOfCompletedIterations will be 1. This is exactly the set of intermediate images we want to display,
  // because the 0th intermediate images are the original inputs.
  if(this->chkLive->isChecked())
    {
    std::cout << "Switching to live iteration." << std::endl;
    //this->IterationToDisplay = this->Inpainting.GetNumberOfCompletedIterations();
    this->IterationToDisplay = this->Inpainting->GetNumberOfCompletedIterations() - 1;
    //std::cout << "Switch to display iteration " << this->IterationToDisplay << std::endl;
    ChangeDisplayedIteration();

    Refresh();
    }
  else
    {
    std::stringstream ss;
    ss << this->IterationToDisplay << " out of " << this->Inpainting->GetNumberOfCompletedIterations();
    this->lblCurrentIteration->setText(ss.str().c_str());
    }

  LeaveFunction("Leave IterationComplete()");
}

void PatchBasedInpaintingGUI::SetupForwardLookingTable()
{
  EnterFunction("SetupForwardLookingTable()");
  if(this->IterationToDisplay < 1)
    {
    //std::cout << "Can only display result patch for iterations > 0." << std::endl;
    this->ForwardLookModel->SetIterationToDisplay(0);
    this->ForwardLookModel->Refresh();
    return;
    }

  this->ForwardLookModel->SetIterationToDisplay(this->IterationToDisplay);
  this->ForwardLookModel->SetPatchDisplaySize(this->PatchDisplaySize);
  this->ForwardLookModel->Refresh();

  this->SourcePatchToDisplayId = 0;

  this->ForwardLookTableView->setColumnWidth(0, this->PatchDisplaySize);
  this->ForwardLookTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  this->ForwardLookTableView->verticalHeader()->setDefaultSectionSize(this->PatchDisplaySize);
  LeaveFunction("SetupForwardLookingTable()");
}

void PatchBasedInpaintingGUI::ChangeDisplayedTopPatch()
{
  EnterFunction("ChangeDisplayedTopPatch()");

  if(this->IterationRecords[this->IterationToDisplay].PotentialPairSets.size() == 0)
    {
    LeaveFunction("ChangeDisplayedTopPatch()");
    return;
    }

  this->SourcePatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->ForwardLookToDisplayId][this->SourcePatchToDisplayId].SourcePatch;
  this->TargetPatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->ForwardLookToDisplayId].TargetPatch;

  DisplaySourcePatch();
  DisplayResultPatch();

  HighlightSourcePatches();

  LeaveFunction("ChangeDisplayedTopPatch()");
}

void PatchBasedInpaintingGUI::ChangeDisplayedForwardLookPatch()
{
  EnterFunction("ChangeDisplayedForwardLookPatch()");

  if(this->IterationRecords[this->IterationToDisplay].PotentialPairSets.size() == 0)
    {
    LeaveFunction("ChangeDisplayedForwardLookPatch()");
    return;
    }
  this->TargetPatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->ForwardLookToDisplayId].TargetPatch;
  DisplayTargetPatch();

  // Once the target patch is set, setup the TopPatches table, which will also display the result patch
  SetupTopPatchesTable();
  ChangeDisplayedTopPatch();

  HighlightForwardLookPatches();

  LeaveFunction("ChangeDisplayedForwardLookPatch()");
}

void PatchBasedInpaintingGUI::SetupTopPatchesTable()
{
  EnterFunction("SetupTopPatchesTable()");

  this->TopPatchesModel->SetIterationToDisplay(this->IterationToDisplay);
  this->TopPatchesModel->SetForwardLookToDisplay(this->ForwardLookToDisplayId);
  this->TopPatchesModel->SetPatchDisplaySize(this->PatchDisplaySize);
  this->TopPatchesModel->SetNumberOfTopPatchesToDisplay(this->NumberOfTopPatchesToDisplay);
  this->TopPatchesModel->Refresh();

  this->SourcePatchToDisplayId = 0;
  HighlightSourcePatches();

  DisplaySourcePatch();
  DisplayResultPatch();

  //this->TopPatchesTableView->resizeColumnsToContents();
  //this->TopPatchesTableView->resizeRowsToContents();

  this->TopPatchesTableView->setColumnWidth(0, this->PatchDisplaySize);
  this->TopPatchesTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  this->TopPatchesTableView->verticalHeader()->setDefaultSectionSize(this->PatchDisplaySize);
  LeaveFunction("SetupTopPatchesTable()");
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

void PatchBasedInpaintingGUI::SetParametersFromGUI()
{
  //this->Inpainting->GetClusterColors()->SetNumberOfColors(this->txtNumberOfBins->text().toUInt());
}

void PatchBasedInpaintingGUI::SetCompareImageFromGUI()
{
  if(Helpers::StringsMatch(this->cmbCompareImage->currentText().toStdString(), "Original"))
    {
    //this->Inpainting->SetCompareToOriginal();
    }
  else if(Helpers::StringsMatch(this->cmbCompareImage->currentText().toStdString(), "Blurred"))
    {
    //this->Inpainting->SetCompareToBlurred();
    }
  else if(Helpers::StringsMatch(this->cmbCompareImage->currentText().toStdString(), "CIELab"))
    {
    //this->Inpainting->SetCompareToCIELAB();
    }
}

void PatchBasedInpaintingGUI::SetComparisonFunctionsFromGUI()
{
  this->Inpainting->GetPatchCompare()->FunctionsToCompute.clear();
  if(this->chkCompareFull->isChecked())
    {
    this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,
                                                                                  this->Inpainting->GetPatchCompare(),_1));
    }
  if(this->chkCompareColor->isChecked())
    {
    this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchColorDifference,
                                                                                  this->Inpainting->GetPatchCompare(),_1));
    }
  if(this->chkCompareDepth->isChecked())
    {
    this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchDepthDifference,
                                                                                  this->Inpainting->GetPatchCompare(),_1));
    }
//   if(this->chkCompareMembership->isChecked())
//     {
//     this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchMembershipDifference,
//                                                                                   this->Inpainting->GetPatchCompare(),_1));
//     }
  if(this->chkCompareHistogramIntersection->isChecked())
    {
    this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchHistogramIntersection,
                                                                                  this->Inpainting->GetPatchCompare(),_1));
    }
}

void PatchBasedInpaintingGUI::SetSortFunctionFromGUI()
{
  if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Full Difference"))
    {
    this->Inpainting->PatchSortFunction = new SortByDifference(PatchPair::AverageAbsoluteDifference, PatchSortFunctor::ASCENDING);
    }
  else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Color Difference"))
    {
    this->Inpainting->PatchSortFunction = new SortByDifference(PatchPair::ColorDifference, PatchSortFunctor::ASCENDING);
    }
  else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Depth Difference"))
    {
    this->Inpainting->PatchSortFunction = new SortByDifference(PatchPair::DepthDifference, PatchSortFunctor::ASCENDING);
    }
  else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Depth + Color Difference"))
    {
    this->Inpainting->PatchSortFunction = new SortByDepthAndColor(PatchPair::CombinedDifference);
    }
  else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Histogram Intersection"))
    {
    this->Inpainting->PatchSortFunction = new SortByDifference(PatchPair::HistogramIntersection, PatchSortFunctor::DESCENDING);
    }
  else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Membership Difference"))
    {
    this->Inpainting->PatchSortFunction = new SortByDifference(PatchPair::MembershipDifference, PatchSortFunctor::DESCENDING);
    }
}

void PatchBasedInpaintingGUI::SetDepthColorLambdaFromGUI()
{
  SortByDepthAndColor* functor = new SortByDepthAndColor(PatchPair::ColorDifference);
  functor->DepthColorLambda = static_cast<float>(sldDepthColorLambda->value())/100.0f;

  this->Inpainting->PatchSortFunction = functor;

  std::cout << "DepthColorLambda set to " << functor->DepthColorLambda << std::endl;
}

void PatchBasedInpaintingGUI::SetPriorityFromGUI()
{
  if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Manual"))
    {
    this->Inpainting->SetPriorityFunction<PriorityManual>();
    if(!ImageExists(this->ImageInputs, "ManualPriority"))
      {
      std::cerr << "Must have a ManualPriority input image to use this priority mode!" << std::endl;
      exit(-1);
      }
    std::cerr << "Setting PriorityManual not yet implemented!" << std::endl; // TODO
    exit(-1);
    //reinterpret_cast<PriorityManual*>(this->Inpainting->GetPriorityFunction())->SetManualPriorityImage(this->InputImages.FindImageByName("ManualPriority").Image.GetPointer());
    }
  else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "OnionPeel"))
    {
    this->Inpainting->SetPriorityFunction<PriorityOnionPeel>();
    }
  else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Random"))
    {
    this->Inpainting->SetPriorityFunction<PriorityRandom>();
    }
  else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Depth"))
    {
    this->Inpainting->SetPriorityFunction<PriorityDepth>();
    }
  else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Criminisi"))
    {
    this->Inpainting->SetPriorityFunction<PriorityCriminisi>();
    }

  // Add priority images to save and display models
  std::vector<NamedVTKImage> namedImages = this->Inpainting->GetPriorityFunction()->GetNamedImages();

  for(unsigned int i = 0; i < namedImages.size(); ++i)
    {
    std::cout << "SetPriorityFromGUI: Adding " << namedImages[i].Name << std::endl;
    
    }

}

void PatchBasedInpaintingGUI::AddImageInput(const ImageInput& imageInput)
{
  this->ImageInputs.push_back(imageInput);
}

void PatchBasedInpaintingGUI::OpenInputImages()
{
  for(int i = 0; i < this->ImageInputs.size(); ++i)
    {
    typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
    ImageReaderType::Pointer reader = ImageReaderType::New();
    reader->SetFileName(this->ImageInputs[i].FileName.toStdString());
    reader->Update();
    this->InputImages.push_back(NamedITKImage(reader->GetOutput(), false, this->ImageInputs[i].Name.toStdString()));
    }
}

void PatchBasedInpaintingGUI::SetupSaveModel()
{
  this->ModelSave->Clear();
  for(unsigned int i = 0; i < this->InputImages.size(); ++i)
    {
    this->ModelSave->Add(this->InputImages[i].Name.c_str(), Qt::Checked);
    }

  std::vector<std::string> priorityImageNames;
  if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Manual"))
    {
    priorityImageNames = PriorityManual::GetImageNames();
    }
  else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "OnionPeel"))
    {
    priorityImageNames = PriorityOnionPeel::GetImageNames();
    }
  else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Random"))
    {
    priorityImageNames = PriorityRandom::GetImageNames();
    }
  else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Depth"))
    {
    priorityImageNames = PriorityDepth::GetImageNames();
    }
  else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Criminisi"))
    {
    priorityImageNames = PriorityCriminisi::GetImageNames();
    }

  for(unsigned int i = 0; i < priorityImageNames.size(); ++i)
    {
    this->ModelSave->Add(priorityImageNames[i].c_str(), Qt::Checked);
    }

}
