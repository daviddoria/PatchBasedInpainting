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
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"
#include "ModelView/ListModelDisplay.h"
#include "ModelView/ListModelSave.h"
#include "Mask.h"
#include "PatchSorting.h"
#include "PixmapDelegate.h"
#include "PriorityFactory.h"
#include "PriorityOnionPeel.h"
#include "PriorityRandom.h"
#include "SelfPatchCompare.h"
#include "Types.h"

void PatchBasedInpaintingGUI::DefaultConstructor()
{
  // This function is called by both constructors. This avoid code duplication.
  EnterFunction("PatchBasedInpaintingGUI::DefaultConstructor()");

  this->Inpainting = NULL;
  this->RecordToDisplay = NULL;

  this->setupUi(this);

  SetupScenes();

  SetupToolbar();

  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  this->UserImage = FloatVectorImageType::New();
  
  this->UserPatch = std::make_shared<MovablePatch>(this->Settings.PatchRadius, this->Renderer, this->gfxUserPatch);
  
  this->RecordViewer = std::make_shared<InpaintingIterationRecordViewer>(this->Renderer);

  // TODO set the image to use in the MovablePatch
  //QImage userPatch = HelpersQt::GetQImage<FloatVectorImageType>(dynamic_cast<FloatVectorImageType*>(this->IterationRecords[this->IterationToDisplay].GetImageByName("Image").Image.GetPointer()),
  //                                                              this->UserPatchRegion, this->ImageDisplayStyle);

  this->Camera = new ImageCamera(this->Renderer);

  this->UserMaskImage = Mask::New();

  SetupComputationThread();

  SetupConnections();

  SetProgressBarToMarquee();

  InitializeGUIElements();

  SetupValidators();

  SetupImageModels();

  LeaveFunction("PatchBasedInpaintingGUI::DefaultConstructor()");
}

void PatchBasedInpaintingGUI::SetupImageModels()
{
  this->ModelSave = QSharedPointer<ListModelSave>(new ListModelSave);
  this->listViewSave->setModel(this->ModelSave.data());

  this->ModelDisplay = QSharedPointer<ListModelDisplay>(new ListModelDisplay);
  this->listViewDisplay->setModel(this->ModelDisplay.data());
  connect (this->listViewDisplay, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_ChangeDisplayedImages(QModelIndex)));

  this->ModelImages = QSharedPointer<TableModelImageInput>(new TableModelImageInput);
  this->ModelImages->setItems(&this->ImageInputs);
  this->tableViewImages->setModel(this->ModelImages.data());
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

void PatchBasedInpaintingGUI::SetupValidators()
{
  this->IntValidator = new QIntValidator(0, 10000, this);
  this->txtPatchRadius->setValidator(this->IntValidator);
  this->txtNumberOfTopPatchesToSave->setValidator(this->IntValidator);
  this->txtNumberOfForwardLook->setValidator(this->IntValidator);
  this->txtNumberOfTopPatchesToDisplay->setValidator(this->IntValidator);

  this->IterationValidator = new QIntValidator(0, 0, this);
  this->txtGoToIteration->setValidator(this->IterationValidator);
}

void PatchBasedInpaintingGUI::SetProgressBarToMarquee()
{
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();
}

void PatchBasedInpaintingGUI::ConnectForwardLookModelToView()
{
  this->ForwardLookModel = QSharedPointer<TableModelForwardLook>(new TableModelForwardLook(this, this->IterationRecords, this->RecordViewer->GetImageDisplayStyle()));
  this->ForwardLookTableView->setModel(this->ForwardLookModel.data());
  this->ForwardLookTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  PixmapDelegate* forwardLookPixmapDelegate = new PixmapDelegate;
  this->ForwardLookTableView->setItemDelegateForColumn(0, forwardLookPixmapDelegate);

  this->connect(this->ForwardLookTableView->selectionModel(), SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )),
                SLOT(slot_ForwardLookTableView_changed(const QModelIndex & , const QModelIndex & )));
}

void PatchBasedInpaintingGUI::ConnectTopPatchesModelToView()
{
  this->TopPatchesModel = QSharedPointer<TableModelTopPatches>(new TableModelTopPatches(this, this->IterationRecords, this->RecordViewer->GetImageDisplayStyle()));
  this->TopPatchesTableView->setModel(this->TopPatchesModel.data());
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
  brush.setColor(this->SceneBackground);
  
  this->TargetPatchScene = new QGraphicsScene();
  this->TargetPatchScene->setBackgroundBrush(brush);
  this->gfxTarget->setScene(TargetPatchScene);

  this->SourcePatchScene = new QGraphicsScene();
  this->SourcePatchScene->setBackgroundBrush(brush);
  this->gfxSource->setScene(SourcePatchScene);

  this->ResultPatchScene = new QGraphicsScene();
  this->ResultPatchScene->setBackgroundBrush(brush);
  this->gfxResult->setScene(ResultPatchScene);
}

void PatchBasedInpaintingGUI::SetupConnections()
{

  connect(this->ComputationThread, SIGNAL(started()), this->InpaintingComputation, SLOT(start()));
  connect(this->InpaintingComputation, SIGNAL(InpaintingComplete()), this->ComputationThread, SLOT(quit()));
  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair*)), this->ComputationThread, SLOT(quit()));

  connect(this->ComputationThread, SIGNAL(started()), this, SLOT(slot_StartProgress()), Qt::QueuedConnection);
  connect(this->InpaintingComputation, SIGNAL(InpaintingComplete()), this, SLOT(slot_StopProgress()), Qt::QueuedConnection);
  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair*)), this, SLOT(slot_StopProgress()), Qt::QueuedConnection);

  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair*)),
          this, SLOT(slot_IterationComplete(const PatchPair*)), Qt::BlockingQueuedConnection);

  ConnectForwardLookModelToView();
  ConnectTopPatchesModelToView();
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

void PatchBasedInpaintingGUI::OpenMask(const std::string& fileName, const bool inverted)
{
  //std::cout << "OpenMask()" << std::endl;
  typedef itk::ImageFileReader<Mask> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  ITKHelpers::DeepCopy<Mask>(reader->GetOutput(), this->UserMaskImage);
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

  ITKHelpers::DeepCopy<FloatVectorImageType>(reader->GetOutput(), this->UserImage);

  //std::cout << "UserImage region: " << this->UserImage->GetLargestPossibleRegion() << std::endl;

  //this->Canvas->DisplayImage(this->UserImage);

  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();

  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);



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
  // The following are valid for all iterations
  if(this->chkDisplayUserPatch->isChecked())
    {
    this->UserPatch->Display();
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

//   this->Canvas->UsedPatchPairLayer.ImageSlice->SetVisibility(this->chkHighlightUsedPatches->isChecked());
// 
//   this->Canvas->ForwardLookPatchLayer.ImageSlice->SetVisibility(this->chkDisplayForwardLookPatchLocations->isChecked());
//   if(this->chkDisplayForwardLookPatchLocations->isChecked())
//     {
//     HighlightForwardLookPatches();
//     }
// 
//   this->Canvas->SourcePatchLayer.ImageSlice->SetVisibility(this->chkDisplaySourcePatchLocations->isChecked());
//   if(this->chkDisplaySourcePatchLocations->isChecked())
//     {
//     HighlightSourcePatches();
//     }

  this->qvtkWidget->GetRenderWindow()->Render();
  LeaveFunction("RefreshVTK()");
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
  EnterFunction("DisplaySourcePatch()");

  if(!this->RecordToDisplay)
    {
    LeaveFunction("DisplaySourcePatch()");
    return;
    }

  FloatVectorImageType::Pointer currentImage = dynamic_cast<FloatVectorImageType*>(this->RecordToDisplay->GetImageByName("Image").Image);

  // TODO: Fix this
//   QImage sourceImage = HelpersQt::GetQImage<FloatVectorImageType>(currentImage, this->SourcePatchToDisplay.Region, this->ImageDisplayStyle);
//   //sourceImage = HelpersQt::FitToGraphicsView(sourceImage, gfxSource);
//   QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));
//   gfxSource->fitInView(item);
//   LeaveFunction("DisplaySourcePatch()");
}

void PatchBasedInpaintingGUI::DisplayTargetPatch()
{
  // We use the previous image and previous mask, but the current PotentialPairSets, as these are the sets that were used to get to this state.
  EnterFunction("DisplayTargetPatch()");

  // The last iteration record will not have any potential patches, because there is nothing left to inpaint!
  if(!RecordToDisplay)
    {
    LeaveFunction("DisplayTargetPatch()");
    return;
    }
  FloatVectorImageType::Pointer currentImage = dynamic_cast<FloatVectorImageType*>(this->RecordToDisplay->GetImageByName("Image").Image);

  // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration
  // (as the current mask has been cleared where the target patch was copied).
  //Mask::Pointer currentMask = dynamic_cast<Mask*>(this->RecordToDisplay->Images.FindImageByName("Mask").Image.GetPointer());

  // Target
  // TODO: Fix this.
//   QImage targetImage = HelpersQt::GetQImage<FloatVectorImageType>(currentImage, this->TargetPatchToDisplay.GetRegion(), this->ImageDisplayStyle);
// 
//   //targetImage = HelpersQt::FitToGraphicsView(targetImage, gfxTarget);
//   QGraphicsPixmapItem* item = this->TargetPatchScene->addPixmap(QPixmap::fromImage(targetImage));
//   gfxTarget->fitInView(item);
  LeaveFunction("DisplayTargetPatch()");

}

void PatchBasedInpaintingGUI::DisplayResultPatch()
{
  EnterFunction("DisplayResultPatch()");

  if(!RecordToDisplay)
    {
    LeaveFunction("DisplayResultPatch()");
    return;
    }

  FloatVectorImageType::Pointer currentImage = dynamic_cast<FloatVectorImageType*>(this->RecordToDisplay->GetImageByName("Image").Image);

  // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration
  // (as the current mask has been cleared where the target patch was copied).
  Mask::Pointer currentMask = dynamic_cast<Mask*>(this->RecordToDisplay->GetImageByName("Mask").Image);

  itk::Size<2> regionSize = this->Inpainting->GetPatchSize();

  QImage qimage(regionSize[0], regionSize[1], QImage::Format_RGB888);

  // TODO: Fix this
//   itk::ImageRegionIterator<FloatVectorImageType> sourceIterator(currentImage, this->SourcePatchToDisplay.GetRegion());
//   itk::ImageRegionIterator<FloatVectorImageType> targetIterator(currentImage, this->TargetPatchToDisplay.GetRegion());
//   itk::ImageRegionIterator<Mask> maskIterator(currentMask, this->TargetPatchToDisplay.GetRegion());
// 
//   FloatVectorImageType::Pointer resultPatch = FloatVectorImageType::New();
//   resultPatch->SetNumberOfComponentsPerPixel(currentImage->GetNumberOfComponentsPerPixel());
//   itk::Size<2> patchSize = ITKHelpers::SizeFromRadius(this->Settings.PatchRadius);
//   itk::ImageRegion<2> region;
//   region.SetSize(patchSize);
//   resultPatch->SetRegions(region);
//   resultPatch->Allocate();
// 
//   while(!maskIterator.IsAtEnd())
//     {
//     FloatVectorImageType::PixelType pixel;
// 
//     if(currentMask->IsHole(maskIterator.GetIndex()))
//       {
//       pixel = sourceIterator.Get();
//       }
//     else
//       {
//       pixel = targetIterator.Get();
//       }

//     itk::Offset<2> offset = sourceIterator.GetIndex() - this->SourcePatchToDisplay.GetRegion().GetIndex();
//     itk::Index<2> offsetIndex;
//     offsetIndex[0] = offset[0];
//     offsetIndex[1] = offset[1];
//     resultPatch->SetPixel(offsetIndex, pixel);

//     ++sourceIterator;
//     ++targetIterator;
//     ++maskIterator;
//     }

  // Color the center pixel
  //qimage.setPixel(regionSize[0]/2, regionSize[1]/2, this->CenterPixelColor.rgb());

//   qimage = HelpersQt::GetQImage<FloatVectorImageType>(resultPatch, resultPatch->GetLargestPossibleRegion(), this->ImageDisplayStyle);
// 
//   //qimage = HelpersQt::FitToGraphicsView(qimage, gfxResult);
//   this->ResultPatchScene->clear();
//   QGraphicsPixmapItem* item = this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));
//   gfxResult->fitInView(item);
//   //this->ResultPatchScene->addPixmap(QPixmap());
//   //std::cout << "Set result patch." << std::endl;
//   LeaveFunction("DisplayResultPatch()");
}

void PatchBasedInpaintingGUI::DisplayUsedPatches()
{
  EnterFunction("DisplayUsedPatches()");

  // There are no patches used in the 0th iteration (initial conditions) so it doesn't make sense to display them.
  // Instead we display blank images.
  if(this->RecordDisplayState.Iteration < 1)
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
}

void PatchBasedInpaintingGUI::DisplayUsedPatchInformation()
{
  EnterFunction("DisplayUsedPatchInformation()");

  if(this->RecordDisplayState.Iteration < 1)
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
}

void PatchBasedInpaintingGUI::ChangeDisplayedIteration()
{
  // This should be called only when the iteration actually changed.
  // TODO: Fix this
/*
  EnterFunction("ChangeDisplayedIteration()");

  if(this->RecordDisplayState.Iteration > this->IterationRecords.size())
    {
    std::cout << "this->IterationToDisplay > this->IterationRecords.size()" << std::endl;
    std::cout << "this->IterationToDisplay: " << this->RecordDisplayState.Iteration << std::endl;
    std::cout << "this->IterationRecords.size(): " << this->IterationRecords.size() << std::endl;
    this->RecordToDisplay = NULL;
    LeaveFunction("ChangeDisplayedIteration()");
    return;
    }

  // If there are no PotentialPairSets, we can't display them.
  if(this->IterationRecords[this->RecordDisplayState.Iteration].PotentialPairSets.size() == 0)
    {
    LeaveFunction("ChangeDisplayedIteration()");
    return;
    }

  this->RecordToDisplay = &IterationRecords[this->RecordDisplayState.Iteration];

  this->ModelDisplay->SetIterationRecord(this->RecordToDisplay);

  this->SourcePatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->RecordDisplayState.ForwardLookId][this->RecordDisplayState.SourcePatchId].SourcePatch;
  this->TargetPatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->RecordDisplayState.ForwardLookId].TargetPatch;

  std::stringstream ss;
  ss << this->RecordDisplayState.Iteration << " out of " << this->Inpainting->GetNumberOfCompletedIterations();
  this->lblCurrentIteration->setText(ss.str().c_str());

  if(this->RecordDisplayState.Iteration > 0)
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
  LeaveFunction("ChangeDisplayedIteration()");*/
}

void PatchBasedInpaintingGUI::CreateInitialRecord()
{
  EnterFunction("CreateInitialRecord()");

  this->IterationRecords.clear();

  InpaintingIterationRecord iterationRecord;

  //NamedITKImage::Pointer image = dynamic_cast<NamedITKImage*>(FloatVectorImageType::New().GetPointer());
  FloatVectorImageType::Pointer image = FloatVectorImageType::New();
  ITKHelpers::DeepCopy<FloatVectorImageType>(this->UserImage, image);
  NamedITKImage namedImage;
  namedImage.Image = image;
  namedImage.Name = "Image";
  iterationRecord.AddImage(namedImage);

  //Helpers::DeepCopy<FloatVectorImageType>(this->UserImage, dynamic_cast<FloatVectorImageType*>(iterationRecord.Images.FindImageByName("Image")));
  //Helpers::DeepCopy<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), stack.Image);

  Mask::Pointer maskImage = Mask::New();
  ITKHelpers::DeepCopy<Mask>(this->UserMaskImage, maskImage);
  NamedITKImage namedMaskImage;
  namedMaskImage.Name = "Mask";
  namedMaskImage.Image = maskImage;
  iterationRecord.AddImage(namedMaskImage);
  //Helpers::DeepCopy<Mask>(this->UserMaskImage, dynamic_cast<Mask*>(iterationRecord.Images.FindImageByName("Mask")));
  //Helpers::DeepCopy<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), stack.Boundary);

  FloatScalarImageType::Pointer priorityImage = FloatScalarImageType::New();
  ITKHelpers::DeepCopy<FloatScalarImageType>(this->Inpainting->GetPriorityFunction()->GetPriorityImage(), priorityImage);
  NamedITKImage namedPriorityImage;
  namedPriorityImage.Name = "Priority";
  namedPriorityImage.Image = priorityImage;
  iterationRecord.AddImage(namedPriorityImage);

  //Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetPriorityFunction()->GetPriorityImage(), dynamic_cast<FloatScalarImageType*>(iterationRecord.Images.FindImageByName("Priority")));

  this->IterationRecords.push_back(iterationRecord);

  this->qvtkWidget->GetRenderWindow()->Render();
  if(this->IterationRecords.size() != 1)
    {
    throw std::runtime_error("this->IterationRecords.size() != 1");
    }
  LeaveFunction("CreateInitialRecord()");
}

void PatchBasedInpaintingGUI::IterationComplete(const PatchPair& usedPatchPair)
{
  EnterFunction("IterationComplete()");

  // TODO: The interfaces used in this function have changed.
  
  InpaintingIterationRecord iterationRecord;
  for(unsigned int i = 0; i < this->Inpainting->GetImagesToUpdate().size(); ++i)
    {
    std::cout << "Updating image " << i << std::endl;

    ITKHelpers::OutputImageType(this->Inpainting->GetImagesToUpdate()[i]);
    itk::ImageBase<2>::Pointer newImage = ITKHelpers::CreateImageWithSameType(this->Inpainting->GetImagesToUpdate()[i]);
    ITKHelpers::OutputImageType(newImage);
    // TODO: Make this deep copy work
    //ITKHelpers::DeepCopy(this->Inpainting->GetImagesToUpdate()[i], newImage);
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

//   if(this->chkRecordSteps->isChecked())
//     {
//     std::cout << "Recording step." << std::endl;
// 
//     // Chop to the desired length
//     for(unsigned int i = 0; i < this->Inpainting->GetPotentialCandidatePairsReference().size(); ++i)
//       {
//       unsigned int numberToKeep = std::min(this->Inpainting->GetPotentialCandidatePairsReference()[i].GetNumberOfSourcePatches(),
//                                            this->Settings.NumberOfTopPatchesToSave);
//       //std::cout << "numberToKeep: " << numberToKeep << std::endl;
// //       this->Inpainting->GetPotentialCandidatePairsReference()[i].erase(this->Inpainting->GetPotentialCandidatePairsReference()[i].begin() + numberToKeep,
// //                                                                        this->Inpainting->GetPotentialCandidatePairsReference()[i].end());
//       }

    // Add the patch pairs to the new record. This will mean the pairs are 1 record out of sync with the images. The interpretation would be:
    // "These are the patches that were considered to get here".
    //iterationRecord.PotentialPairSets = this->Inpainting.GetPotentialCandidatePairs();

    // Add the patch pairs to the previous record. The interpretation is "these patches are considered here, to produce the next image".
    // There should always be a previous record, because an initial record is created for the initial state.
    //this->IterationRecords[this->IterationRecords.size() - 1].PotentialPairSets = this->Inpainting->GetPotentialCandidatePairs();
    //std::cout << "iterationRecord.PotentialPairSets: " << iterationRecord.PotentialPairSets.size() << std::endl;
    //}

}

void PatchBasedInpaintingGUI::SetupForwardLookingTable()
{
  EnterFunction("SetupForwardLookingTable()");
  if(this->RecordDisplayState.Iteration < 1)
    {
    //std::cout << "Can only display result patch for iterations > 0." << std::endl;
    this->ForwardLookModel->SetIterationToDisplay(0);
    this->ForwardLookModel->Refresh();
    return;
    }

  this->ForwardLookModel->SetIterationToDisplay(this->RecordDisplayState.Iteration);
  this->ForwardLookModel->SetPatchDisplaySize(this->Settings.PatchDisplaySize);
  this->ForwardLookModel->Refresh();

  this->RecordDisplayState.SourcePatchId = 0;

  this->ForwardLookTableView->setColumnWidth(0, this->Settings.PatchDisplaySize);
  this->ForwardLookTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  this->ForwardLookTableView->verticalHeader()->setDefaultSectionSize(this->Settings.PatchDisplaySize);
  LeaveFunction("SetupForwardLookingTable()");
}

void PatchBasedInpaintingGUI::ChangeDisplayedTopPatch()
{
  EnterFunction("ChangeDisplayedTopPatch()");

  if(this->IterationRecords[this->RecordDisplayState.Iteration].PotentialPairSets.size() == 0)
    {
    LeaveFunction("ChangeDisplayedTopPatch()");
    return;
    }

    // TODO: Fix this
//   this->SourcePatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->RecordDisplayState.ForwardLookId][this->RecordDisplayState.SourcePatchId].SourcePatch;
//   this->TargetPatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->RecordDisplayState.ForwardLookId].TargetPatch;
// 
//   DisplaySourcePatch();
//   DisplayResultPatch();
// 
//   HighlightSourcePatches();

  LeaveFunction("ChangeDisplayedTopPatch()");
}

void PatchBasedInpaintingGUI::ChangeDisplayedForwardLookPatch()
{
  EnterFunction("ChangeDisplayedForwardLookPatch()");

  if(this->IterationRecords[this->RecordDisplayState.Iteration].PotentialPairSets.size() == 0)
    {
    LeaveFunction("ChangeDisplayedForwardLookPatch()");
    return;
    }
    // TODO: Fix this
//   this->TargetPatchToDisplay = this->RecordToDisplay->PotentialPairSets[this->RecordDisplayState.ForwardLookId].TargetPatch;
//   DisplayTargetPatch();
// 
//   // Once the target patch is set, setup the TopPatches table, which will also display the result patch
//   SetupTopPatchesTable();
//   ChangeDisplayedTopPatch();
// 
//   HighlightForwardLookPatches();

  LeaveFunction("ChangeDisplayedForwardLookPatch()");
}

void PatchBasedInpaintingGUI::SetupTopPatchesTable()
{
  EnterFunction("SetupTopPatchesTable()");

  this->TopPatchesModel->SetIterationToDisplay(this->RecordDisplayState.Iteration);
  this->TopPatchesModel->SetForwardLookToDisplay(this->RecordDisplayState.ForwardLookId);
  this->TopPatchesModel->SetPatchDisplaySize(this->Settings.PatchDisplaySize);
  this->TopPatchesModel->SetNumberOfTopPatchesToDisplay(this->Settings.NumberOfTopPatchesToDisplay);
  this->TopPatchesModel->Refresh();

  this->RecordDisplayState.SourcePatchId = 0;
  //HighlightSourcePatches();

  DisplaySourcePatch();
  DisplayResultPatch();

  //this->TopPatchesTableView->resizeColumnsToContents();
  //this->TopPatchesTableView->resizeRowsToContents();

  this->TopPatchesTableView->setColumnWidth(0, this->Settings.PatchDisplaySize);
  this->TopPatchesTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  this->TopPatchesTableView->verticalHeader()->setDefaultSectionSize(this->Settings.PatchDisplaySize);
  LeaveFunction("SetupTopPatchesTable()");
}

void PatchBasedInpaintingGUI::InitializeGUIElements()
{
  on_chkLive_clicked();

  this->Settings.PatchRadius = this->txtPatchRadius->text().toUInt();

  this->Settings.NumberOfTopPatchesToSave = this->txtNumberOfTopPatchesToSave->text().toUInt();

  this->Settings.NumberOfForwardLook = this->txtNumberOfForwardLook->text().toUInt();

  this->Settings.NumberOfTopPatchesToDisplay = this->txtNumberOfTopPatchesToDisplay->text().toUInt();
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
  // TODO: This interface has changed significantly
//   this->Inpainting->GetPatchCompare()->FunctionsToCompute.clear();
//   if(this->chkCompareFull->isChecked())
//     {
//     this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,
//                                                                                   this->Inpainting->GetPatchCompare(),_1));
//     }
//   if(this->chkCompareColor->isChecked())
//     {
//     this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchColorDifference,
//                                                                                   this->Inpainting->GetPatchCompare(),_1));
//     }
//   if(this->chkCompareDepth->isChecked())
//     {
//     this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchDepthDifference,
//                                                                                   this->Inpainting->GetPatchCompare(),_1));
//     }
// //   if(this->chkCompareMembership->isChecked())
// //     {
// //     this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchMembershipDifference,
// //                                                                                   this->Inpainting->GetPatchCompare(),_1));
// //     }
//   if(this->chkCompareHistogramIntersection->isChecked())
//     {
//     this->Inpainting->GetPatchCompare()->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchHistogramIntersection,
//                                                                                   this->Inpainting->GetPatchCompare(),_1));
//     }
}

void PatchBasedInpaintingGUI::SetSortFunctionFromGUI()
{
  // TODO: This interface has changed significantly.
//   if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Full Difference"))
//     {
//     this->Inpainting->PatchSortFunction = std::make_shared<SortByDifference>(PatchPair::AverageAbsoluteDifference, PatchSortFunctor::ASCENDING);
//     }
//   else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Color Difference"))
//     {
//     this->Inpainting->PatchSortFunction = std::make_shared<SortByDifference>(PatchPair::ColorDifference, PatchSortFunctor::ASCENDING);
//     }
//   else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Depth Difference"))
//     {
//     this->Inpainting->PatchSortFunction = std::make_shared<SortByDifference>(PatchPair::DepthDifference, PatchSortFunctor::ASCENDING);
//     }
//   else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Depth + Color Difference"))
//     {
//     this->Inpainting->PatchSortFunction = std::make_shared<SortByDepthAndColor>(PatchPair::CombinedDifference);
//     }
//   else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Histogram Intersection"))
//     {
//     this->Inpainting->PatchSortFunction = std::make_shared<SortByDifference>(PatchPair::HistogramIntersection, PatchSortFunctor::DESCENDING);
//     }
//   else if(Helpers::StringsMatch(this->cmbSortBy->currentText().toStdString(), "Membership Difference"))
//     {
//     this->Inpainting->PatchSortFunction = std::make_shared<SortByDifference>(PatchPair::MembershipDifference, PatchSortFunctor::DESCENDING);
//     }
}

void PatchBasedInpaintingGUI::SetPriorityFromGUI()
{
  this->Inpainting->SetPriorityFunction(this->cmbPriority->currentText().toStdString());
//   if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Manual"))
//     {
//     if(!ImageExists(this->ImageInputs, "ManualPriority"))
//       {
//       std::cerr << "Must have a ManualPriority input image to use this priority mode!" << std::endl;
//       exit(-1);
//       }
//     std::cerr << "Setting PriorityManual not yet implemented!" << std::endl; // TODO
//     exit(-1);
//     //reinterpret_cast<PriorityManual*>(this->Inpainting->GetPriorityFunction())->
  //SetManualPriorityImage(this->InputImages.FindImageByName("ManualPriority").Image.GetPointer());
//     }
  
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
    this->InputImages.push_back(NamedITKImage(reader->GetOutput(), NamedITKImage::SCALARS, this->ImageInputs[i].Name.toStdString()));
    }
}

void PatchBasedInpaintingGUI::SetupSaveModel()
{
  this->ModelSave->Clear();
  for(unsigned int i = 0; i < this->InputImages.size(); ++i)
    {
    this->ModelSave->Add(this->InputImages[i].Name.c_str(), Qt::Checked);
    }

    // TODO: something wrong with this call (compiler error).
//   std::vector<std::string> priorityImageNames = PriorityFactory<PriorityRandom<FloatScalarImageType> >::GetImageNames(this->cmbPriority->currentText().toStdString());
// 
//   for(unsigned int i = 0; i < priorityImageNames.size(); ++i)
//     {
//     this->ModelSave->Add(priorityImageNames[i].c_str(), Qt::Checked);
//     }
}
