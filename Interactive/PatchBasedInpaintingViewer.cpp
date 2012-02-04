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

#include "PatchBasedInpaintingViewer.h"

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

// Custom
#include "FileSelector.h"
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"
#include "Mask.h"
#include "Types.h"

void PatchBasedInpaintingViewer::DefaultConstructor()
{
  // This function is called by both constructors. This avoid code duplication.
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

  this->Camera = new ImageCamera(this->Renderer);

  this->UserMaskImage = Mask::New();

  SetupComputationThread();

  SetupConnections();

  SetProgressBarToMarquee();

  InitializeGUIElements();

  SetupValidators();

  SetupImageModels();

}

void PatchBasedInpaintingViewer::SetupImageModels()
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

void PatchBasedInpaintingViewer::SetupComputationThread()
{
  this->ComputationThread = new QThread;
  this->InpaintingComputation = new InpaintingComputationObject;
  this->InpaintingComputation->moveToThread(this->ComputationThread);
}

void PatchBasedInpaintingViewer::SetupToolbar()
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

void PatchBasedInpaintingViewer::SetupValidators()
{
  this->IntValidator = new QIntValidator(0, 10000, this);
  this->txtPatchRadius->setValidator(this->IntValidator);
  this->txtNumberOfTopPatchesToSave->setValidator(this->IntValidator);
  this->txtNumberOfForwardLook->setValidator(this->IntValidator);
  this->txtNumberOfTopPatchesToDisplay->setValidator(this->IntValidator);

  this->IterationValidator = new QIntValidator(0, 0, this);
  this->txtGoToIteration->setValidator(this->IterationValidator);
}

void PatchBasedInpaintingViewer::SetProgressBarToMarquee()
{
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();
}

void PatchBasedInpaintingViewer::SetupScenes()
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

void PatchBasedInpaintingViewer::SetupConnections()
{

  connect(this->ComputationThread, SIGNAL(started()), this->InpaintingComputation, SLOT(start()));
  connect(this->InpaintingComputation, SIGNAL(InpaintingComplete()), this->ComputationThread, SLOT(quit()));
  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair*)), this->ComputationThread, SLOT(quit()));

  connect(this->ComputationThread, SIGNAL(started()), this, SLOT(slot_StartProgress()), Qt::QueuedConnection);
  connect(this->InpaintingComputation, SIGNAL(InpaintingComplete()), this, SLOT(slot_StopProgress()), Qt::QueuedConnection);
  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair*)), this, SLOT(slot_StopProgress()), Qt::QueuedConnection);

  connect(this->InpaintingComputation, SIGNAL(IterationComplete(const PatchPair*)),
          this, SLOT(slot_IterationComplete(const PatchPair*)), Qt::BlockingQueuedConnection);

}

void PatchBasedInpaintingViewer::showEvent ( QShowEvent * event )
{
  SetupSaveModel();
}

// Default constructor
PatchBasedInpaintingViewer::PatchBasedInpaintingViewer()
{
  DefaultConstructor();
};

PatchBasedInpaintingViewer::PatchBasedInpaintingViewer(const std::string& imageFileName,
                                                       const std::string& maskFileName, const bool debugEnterLeave = false)
{
  this->SetDebugFunctionEnterLeave(debugEnterLeave);

  std::cout << "Image: " << imageFileName << " Mask: " << maskFileName << std::endl;

  DefaultConstructor();

  OpenImage(imageFileName);
  OpenMask(maskFileName, false);

  Initialize();
}

void PatchBasedInpaintingViewer::OpenMask(const std::string& fileName, const bool inverted)
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

  //Helpers::DebugWriteImageConditional<Mask>(this->UserMaskImage, "Debug/InvertedMask.png", this->DebugImages);
}

void PatchBasedInpaintingViewer::UpdateAllImageInputModels()
{
  this->ModelSave->Refresh();
  //this->ModelDisplay->Refresh();
  this->ModelImages->Refresh();
}

void PatchBasedInpaintingViewer::OpenImage(const std::string& fileName)
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

void PatchBasedInpaintingViewer::Reset()
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



// void PatchBasedInpaintingViewer::DisplayBoundary()
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

void PatchBasedInpaintingViewer::RefreshVTK()
{
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
}

void PatchBasedInpaintingViewer::RefreshQt()
{
  ChangeDisplayedTopPatch();
  ChangeDisplayedForwardLookPatch();
  SetupForwardLookingTable();
  SetupTopPatchesTable();
}

void PatchBasedInpaintingViewer::Refresh()
{
  RefreshVTK();
  RefreshQt();
}


void PatchBasedInpaintingViewer::Initialize()
{
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

}

void PatchBasedInpaintingViewer::DisplaySourcePatch()
{
  if(!this->RecordToDisplay)
    {
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
  // We use the previous image and previous mask, but the current PotentialPairSets,
  // as these are the sets that were used to get to this state.

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
}

void PatchBasedInpaintingViewer::DisplayResultPatch()
{
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

}


void PatchBasedInpaintingViewer::DisplayUsedPatchInformation()
{
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
}

void PatchBasedInpaintingViewer::CreateInitialRecord()
{
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
}

void PatchBasedInpaintingViewer::IterationComplete(const PatchPair& usedPatchPair)
{
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

void PatchBasedInpaintingViewer::SetupForwardLookingTable()
{
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
}

void PatchBasedInpaintingViewer::InitializeGUIElements()
{
  on_chkLive_clicked();

  this->Settings.PatchRadius = this->txtPatchRadius->text().toUInt();

  this->Settings.NumberOfTopPatchesToSave = this->txtNumberOfTopPatchesToSave->text().toUInt();

  this->Settings.NumberOfForwardLook = this->txtNumberOfForwardLook->text().toUInt();

  this->Settings.NumberOfTopPatchesToDisplay = this->txtNumberOfTopPatchesToDisplay->text().toUInt();
}

void PatchBasedInpaintingViewer::SetParametersFromGUI()
{
  //this->Inpainting->GetClusterColors()->SetNumberOfColors(this->txtNumberOfBins->text().toUInt());
}
