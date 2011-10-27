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

#include "ui_CriminisiInpainting.h"
#include "Form.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QFileDialog>
#include <QIcon>
#include <QTextEdit>

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
#include "Helpers.h"
#include "InteractorStyleImageNoLevel.h"
#include "Mask.h"
#include "Types.h"

void Form::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();
  
  help->setReadOnly(true);
  help->append("<h1>Criminisi Inpainting</h1>\
  Load an image and a mask. <br/>\
  Set the settings such as patch size. <br/>\
  To do the complete inpainting, click 'Inpaint'.<br/>\
  To do one step of the inpainting, click 'Step'. This will allow you to inspect forward look candidates and each of their top matches.<br/>\
  <p/>");
  help->show();
}

void Form::SetupColors()
{
  this->UsedTargetPatchColor = Qt::red;
  this->UsedSourcePatchColor = Qt::green;
  this->AllForwardLookPatchColor = Qt::darkCyan;
  this->SelectedForwardLookPatchColor = Qt::cyan;
  this->AllSourcePatchColor = Qt::darkMagenta;
  this->SelectedSourcePatchColor = Qt::magenta;
  this->CenterPixelColor = Qt::blue;
  this->MaskColor = Qt::darkGray;
  //this->HoleColor = Qt::gray;
  this->HoleColor.setRgb(255, 153, 0); // Orange
  this->SceneBackgroundColor.setRgb(153, 255, 0); // ?
}
  
void Form::DefaultConstructor()
{
  // This function is called by both constructors. This avoid code duplication.
  this->setupUi(this);

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

  this->IterationToDisplay = 0;

  this->DebugImages = false;
  this->DebugMessages = false;

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");
  QIcon saveIcon = QIcon::fromTheme("document-save");
  
  // Setup toolbar
  actionOpen->setIcon(openIcon);
  this->toolBar->addAction(actionOpen);

  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);

  this->Flipped = false;
  
  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageNoLevel>::New();
    
  // An image with potential target patches outlined
  this->PotentialTargetPatchesImage = UnsignedCharScalarImageType::New();
    
  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);
  
  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);
  this->Renderer->AddViewProp(this->ConfidenceLayer.ImageSlice);
  this->Renderer->AddViewProp(this->ConfidenceMapLayer.ImageSlice);
  this->Renderer->AddViewProp(this->BoundaryLayer.ImageSlice);
  this->Renderer->AddViewProp(this->PriorityLayer.ImageSlice);
  this->Renderer->AddViewProp(this->DataLayer.ImageSlice);
  this->Renderer->AddViewProp(this->MaskLayer.ImageSlice);
  this->Renderer->AddViewProp(this->PotentialPatchesLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedTargetPatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->UsedSourcePatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->AllSourcePatchOutlinesLayer.ImageSlice);
  this->Renderer->AddViewProp(this->AllForwardLookOutlinesLayer.ImageSlice);

  this->BoundaryNormalsLayer.Actor->VisibilityOff(); // If this isn't set, there is an error about "no input" to vtkHedgeHog because the boundary normals are tried to be displayed before they are necessarily populated.
  this->Renderer->AddViewProp(this->BoundaryNormalsLayer.Actor);

  this->IsophoteLayer.Actor->VisibilityOff();
  this->Renderer->AddViewProp(this->IsophoteLayer.Actor);
  
  this->Renderer->AddViewProp(this->SelectedForwardLookOutlineLayer.ImageSlice);// This should be added after AllForwardLookOutlinesLayer.
  this->Renderer->AddViewProp(this->SelectedSourcePatchOutlineLayer.ImageSlice);// This should be added after AllSourcePatchOutlinesLayer.
  
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  
  this->UserImage = FloatVectorImageType::New();
  this->UserMaskImage = Mask::New();
  
  connect(&ComputationThread, SIGNAL(StartProgressSignal()), this, SLOT(StartProgressSlot()), Qt::QueuedConnection);
  connect(&ComputationThread, SIGNAL(StopProgressSignal()), this, SLOT(StopProgressSlot()), Qt::QueuedConnection);
  
  // Using a blocking connection allows everything (computation and drawing) to be performed sequentially which is helpful for debugging, but makes the interface very very choppy.
  // We are assuming that the computation takes longer than the drawing.
  //connect(&ComputationThread, SIGNAL(IterationCompleteSignal()), this, SLOT(IterationCompleteSlot()), Qt::QueuedConnection);
  connect(&ComputationThread, SIGNAL(IterationCompleteSignal()), this, SLOT(IterationCompleteSlot()), Qt::BlockingQueuedConnection);
  
  connect(&ComputationThread, SIGNAL(RefreshSignal()), this, SLOT(RefreshSlot()), Qt::QueuedConnection);

  // Set the progress bar to marquee mode
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();
  
  this->ComputationThread.SetObject(&(this->Inpainting));

  /*
  QPalette forwardLookingPalette = forwardLookingTableWidget->palette();
  forwardLookingPalette.setColor(QPalette::Inactive, QPalette::Window, forwardLookingPalette.color(QPalette::Active, QPalette::Window));

  QPalette topPatchesPalette = topPatchesTableWidget->palette();
  topPatchesPalette.setColor(QPalette::Inactive, QPalette::Window, topPatchesPalette.color(QPalette::Active, QPalette::Window));
  */
}

// Default constructor
Form::Form()
{
  DefaultConstructor();
};

Form::Form(const std::string& imageFileName, const std::string& maskFileName)
{
  DefaultConstructor();
  
  OpenImage(imageFileName);
  OpenMask(maskFileName, false);
  Initialize();
}

void Form::on_chkDebugImages_clicked()
{
  QDir directoryMaker;
  directoryMaker.mkdir("Debug");
  
  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->DebugImages = this->chkDebugImages->isChecked();
  
  DebugMessage<bool>("DebugImages: ", this->DebugImages);
}

void Form::on_chkDebugMessages_clicked()
{
  this->Inpainting.SetDebugMessages(this->chkDebugMessages->isChecked());
  this->DebugMessages = this->chkDebugMessages->isChecked();
}

void Form::on_actionQuit_activated()
{
  exit(0);
}

void Form::on_actionSaveResult_activated()
{
  // Get a filename to save
  QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha)");

  DebugMessage<std::string>("Got filename: ", fileName.toStdString());
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }
    
  Helpers::WriteImage<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), fileName.toStdString());
  
  this->statusBar()->showMessage("Saved result.");
}

void Form::StartProgressSlot()
{
  //std::cout << "Form::StartProgressSlot()" << std::endl;
  // Connected to the StartProgressSignal of the ProgressThread member
  this->progressBar->show();
}

void Form::StopProgressSlot()
{
  //std::cout << "Form::StopProgressSlot()" << std::endl;
  // When the ProgressThread emits the StopProgressSignal, we need to display the result of the segmentation

  this->progressBar->hide();
}

void Form::on_actionOpen_activated()
{
  FileSelector* fileSelector(new FileSelector);
  fileSelector->exec();

  int result = fileSelector->result();
  if(result) // The user clicked 'ok'
    {
    std::cout << "User clicked ok." << std::endl;
    OpenImage(fileSelector->GetImageFileName());
    OpenMask(fileSelector->GetMaskFileName(), fileSelector->IsMaskInverted());
    Initialize();
    }
  else
    {
    std::cout << "User clicked cancel." << std::endl;
    // The user clicked 'cancel' or closed the dialog, do nothing.
    }
}


void Form::OpenMask(const std::string& fileName, const bool inverted)
{
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

  /*
  if(this->UserImage && this->UserMaskImage && this->UserImage->GetLargestPossibleRegion() == this->UserMaskImage->GetLargestPossibleRegion())
    {
    SetupInitialIntermediateImages();
    SetCheckboxVisibility(true);
    }
  else
    {
    SetCheckboxVisibility(false);
    }
  */

  this->UserMaskImage->Cleanup();

  if(inverted)
    {
    this->UserMaskImage->Invert();
    }

  //Helpers::DebugWriteImageConditional<Mask>(this->UserMaskImage, "Debug/InvertedMask.png", this->DebugImages);
}


void Form::OpenImage(const std::string& fileName)
{
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

  //this->Image = reader->GetOutput();
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(reader->GetOutput(), this->UserImage);
  //std::cout << "UserImage region: " << this->UserImage->GetLargestPossibleRegion() << std::endl;

  Helpers::ITKVectorImagetoVTKImage(this->UserImage, this->ImageLayer.ImageData);

  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();

  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);

  /*
  if(this->UserImage && this->UserMaskImage && this->UserImage->GetLargestPossibleRegion() == this->UserMaskImage->GetLargestPossibleRegion())
    {
    SetupInitialIntermediateImages();
    SetCheckboxVisibility(true);
    }
  else
    {
    SetCheckboxVisibility(false);
    }
  */
  this->AllForwardLookOutlinesLayer.ImageData->SetDimensions(this->UserImage->GetLargestPossibleRegion().GetSize()[0],
                                                             this->UserImage->GetLargestPossibleRegion().GetSize()[1], 1);
  this->AllForwardLookOutlinesLayer.ImageData->AllocateScalars();
  this->AllSourcePatchOutlinesLayer.ImageData->SetDimensions(this->UserImage->GetLargestPossibleRegion().GetSize()[0],
                                                             this->UserImage->GetLargestPossibleRegion().GetSize()[1], 1);
  this->AllSourcePatchOutlinesLayer.ImageData->AllocateScalars();

}



void Form::DisplayIsophotes()
{
  if(this->IntermediateImages[this->IterationToDisplay].Isophotes->GetLargestPossibleRegion().GetSize()[0] != 0)
    {
    // Mask the isophotes image with the current boundary, because we only want to display the isophotes we are interested in.
    //FloatVector2ImageType::Pointer normalizedIsophotes = FloatVector2ImageType::New();
    //Helpers::DeepCopy<FloatVector2ImageType>(this->IntermediateImages[this->IterationToDisplay].Isophotes, normalizedIsophotes);
    //Helpers::NormalizeVectorImage(normalizedIsophotes);

    typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType> MaskFilterType;
    typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    //maskFilter->SetInput(normalizedIsophotes);
    maskFilter->SetInput(this->IntermediateImages[this->IterationToDisplay].Isophotes);
    maskFilter->SetMaskImage(this->IntermediateImages[this->IterationToDisplay].Boundary);
    FloatVector2ImageType::PixelType zero;
    zero.Fill(0);
    maskFilter->SetOutsideValue(zero);
    maskFilter->Update();
  
    Helpers::DebugWriteImageConditional<FloatVector2ImageType>(maskFilter->GetOutput(), "Debug/ShowIsophotes.BoundaryIsophotes.mha", this->DebugImages);
    Helpers::DebugWriteImageConditional<UnsignedCharScalarImageType>(this->IntermediateImages[this->IterationToDisplay].Boundary, "Debug/ShowIsophotes.Boundary.mha", this->DebugImages);
    
    Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), this->IsophoteLayer.Vectors);
    
    if(this->DebugImages)
      {
      vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
      writer->SetFileName("Debug/VTKIsophotes.vti");
      writer->SetInputConnection(this->IsophoteLayer.ImageData->GetProducerPort());
      writer->Write();
    
      vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
      polyDataWriter->SetFileName("Debug/VTKIsophotes.vtp");
      polyDataWriter->SetInputConnection(this->IsophoteLayer.Vectors->GetProducerPort());
      polyDataWriter->Write();
      }

    } 
  else
    {
    std::cerr << "Isophotes are not defined!" << std::endl;
    }
}

void Form::DisplayMask()
{
  //vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  //Helpers::ITKScalarImageToScaledVTKImage<Mask>(this->IntermediateImages[this->IterationToDisplay].MaskImage, temp);  
  //Helpers::MakeValidPixelsTransparent(temp, this->MaskLayer.ImageData, 0); // Set the zero pixels of the mask to transparent
  
  this->IntermediateImages[this->IterationToDisplay].MaskImage->MakeVTKImage(this->MaskLayer.ImageData, QColor(Qt::white), this->HoleColor, false, true); // (..., holeTransparent, validTransparent);
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::DisplayConfidence()
{
  vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->IntermediateImages[this->IterationToDisplay].Confidence, temp);  
  Helpers::MakePixelsTransparent(temp, this->ConfidenceLayer.ImageData, 0); // Set the zero pixels to transparent
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::DisplayConfidenceMap()
{
  Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->IntermediateImages[this->IterationToDisplay].ConfidenceMap, this->ConfidenceMapLayer.ImageData);
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::DisplayImage()
{
  Helpers::ITKVectorImagetoVTKImage(this->IntermediateImages[this->IterationToDisplay].Image, this->ImageLayer.ImageData);
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::DisplayBoundary()
{
  Helpers::ITKScalarImageToScaledVTKImage<UnsignedCharScalarImageType>(this->IntermediateImages[this->IterationToDisplay].Boundary, this->BoundaryLayer.ImageData);
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::DisplayPriority()
{
  vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->IntermediateImages[this->IterationToDisplay].Priority, temp);
  Helpers::MakePixelsTransparent(temp, this->PriorityLayer.ImageData, 0); // Set the zero pixels to transparent
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::DisplayData()
{
  vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->IntermediateImages[this->IterationToDisplay].Data, temp);
  Helpers::MakePixelsTransparent(temp, this->DataLayer.ImageData, 0); // Set the zero pixels to transparent
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::RefreshSlot()
{
  DebugMessage("RefreshSlot()");

  Refresh();
}

void Form::DisplayBoundaryNormals()
{
  if(this->Inpainting.GetBoundaryNormalsImage()->GetLargestPossibleRegion().GetSize()[0] != 0)
    {
    Helpers::ConvertNonZeroPixelsToVectors(this->IntermediateImages[this->IterationToDisplay].BoundaryNormals, this->BoundaryNormalsLayer.Vectors);
    this->qvtkWidget->GetRenderWindow()->Render();
  
    if(this->DebugImages)
      {
      std::cout << "Writing boundary normals..." << std::endl;
    
      Helpers::WriteImage<FloatVector2ImageType>(this->Inpainting.GetBoundaryNormalsImage(), "Debug/RefreshSlot.BoundaryNormals.mha");

      Helpers::WriteImageData(this->BoundaryNormalsLayer.ImageData, "Debug/RefreshSlot.VTKBoundaryNormals.vti");

      Helpers::WritePolyData(this->BoundaryNormalsLayer.Vectors, "Debug/RefreshSlot.VTKBoundaryNormals.vtp");
      }
    }  
}

void Form::Refresh()
{
  DebugMessage("Refresh()");

  // The following are valid for all iterations
  this->ImageLayer.ImageSlice->SetVisibility(this->chkImage->isChecked());
  if(this->chkImage->isChecked())
    {
    DisplayImage();
    }

  this->MaskLayer.ImageSlice->SetVisibility(this->chkMask->isChecked());
  if(this->chkMask->isChecked())
    {
    DisplayMask();
    }

  this->ConfidenceMapLayer.ImageSlice->SetVisibility(this->chkConfidenceMap->isChecked());
  if(this->chkConfidenceMap->isChecked())
    {
    DisplayConfidenceMap();
    }

  this->ConfidenceLayer.ImageSlice->SetVisibility(this->chkConfidence->isChecked());
  if(this->chkConfidence->isChecked())
    {
    DisplayConfidence();
    }

  this->PriorityLayer.ImageSlice->SetVisibility(this->chkPriority->isChecked());
  if(this->chkPriority->isChecked())
    {
    DisplayPriority();
    }

  this->BoundaryLayer.ImageSlice->SetVisibility(this->chkBoundary->isChecked());
  if(this->chkBoundary->isChecked())
    {
    DisplayBoundary();
    }

  this->IsophoteLayer.Actor->SetVisibility(this->chkIsophotes->isChecked());
  if(this->chkIsophotes->isChecked())
    {
    DisplayIsophotes();
    }

  this->DataLayer.ImageSlice->SetVisibility(this->chkData->isChecked());
  if(this->chkData->isChecked())
    {
    DisplayData();
    }

  this->BoundaryNormalsLayer.Actor->SetVisibility(this->chkBoundaryNormals->isChecked());
  if(this->chkBoundaryNormals->isChecked())
    {
    DisplayBoundaryNormals();
    }

  this->PotentialPatchesLayer.ImageSlice->SetVisibility(this->chkPotentialPatches->isChecked());
  if(this->chkPotentialPatches->isChecked())
    {
    //Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->Inpainting.GetDataImage(), this->VTKDataImage);
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

}

void Form::on_btnStop_clicked()
{
  this->ComputationThread.StopInpainting();
}

void Form::on_btnReset_clicked()
{
  RefreshSlot();
}
  
void Form::on_btnStep_clicked()
{
  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->Inpainting.SetDebugMessages(this->chkDebugMessages->isChecked());
  this->Inpainting.SetMaxForwardLookPatches(this->txtNumberOfForwardLook->text().toUInt());
  this->Inpainting.SetNumberOfTopPatchesToSave(this->txtNumberOfTopPatches->text().toUInt());
  this->Inpainting.Iterate();
  
  IterationComplete();
}

void Form::on_btnInitialize_clicked()
{
  Initialize();
}

void Form::Initialize()
{
  // Reset some things (this is so that if we want to run another completion it will work normally)

  this->UserMaskImage->ApplyToVectorImage<FloatVectorImageType>(this->UserImage, this->HoleColor);
  
  this->Inpainting.SetPatchRadius(this->txtPatchRadius->text().toUInt());
  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->Inpainting.SetDebugMessages(this->chkDebugMessages->isChecked());
  this->Inpainting.SetMask(this->UserMaskImage);
  this->Inpainting.SetImage(this->UserImage);
  this->Inpainting.Initialize();
  
  SetupInitialIntermediateImages();
  SetCheckboxVisibility(true);
  
  Refresh();
}

void Form::on_btnInpaint_clicked()
{
  DebugMessage("on_btnInpaint_clicked()");
  
  //Initialize();
  
  Refresh();
  
  DebugMessage("Starting ComputationThread...");
  
  this->Inpainting.SetMaxForwardLookPatches(this->txtNumberOfForwardLook->text().toUInt());
  this->Inpainting.SetNumberOfTopPatchesToSave(this->txtNumberOfTopPatches->text().toUInt());
  
  ComputationThread.start();
}


void Form::DebugMessage(const std::string& message)
{
  if(this->DebugMessages)
    {
    std::cout << message << std::endl;
    }
}

void Form::on_btnDisplayPreviousStep_clicked()
{
  if(this->IterationToDisplay > 0)
    {
    this->IterationToDisplay--;
    DebugMessage<unsigned int>("Displaying iteration: ", this->IterationToDisplay);
    ChangeDisplayedIteration();
    }
  else
    {
    DebugMessage("Iteration not changed.");
    }
}

void Form::on_btnDisplayNextStep_clicked()
{
  //std::cout << "IterationToDisplay: " << this->IterationToDisplay
    //        << " Inpainting iteration: " <<  static_cast<int>(this->Inpainting.GetIteration()) << std::endl;
  
  //if(this->IterationToDisplay < this->Inpainting.GetNumberOfCompletedIterations() - 1)
  if(this->IterationToDisplay < this->IntermediateImages.size() - 1)
    {
    this->IterationToDisplay++;
    DebugMessage<unsigned int>("Displaying iteration: ", this->IterationToDisplay);
    ChangeDisplayedIteration();
    }
  else
    {
    DebugMessage("Iteration not changed.");
    }
}

void Form::DisplaySourcePatch(const unsigned int forwardLookId, const unsigned int topPatchId)
{
  DebugMessage("DisplaySourcePatch()");
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display result patch for iterations > 0." << std::endl;
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }

  FloatVectorImageType::Pointer currentImage = this->IntermediateImages[this->IterationToDisplay].Image;

  const CandidatePairs& candidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1][forwardLookId]; // This -1 is because the 0th iteration is the initial condition
  QImage sourceImage = Helpers::GetQImageColor<FloatVectorImageType>(currentImage, candidatePairs[topPatchId].SourcePatch.Region);
  sourceImage = Helpers::FitToGraphicsView(sourceImage, gfxTarget);
  this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));

  //Refresh();
}

void Form::DisplayTargetPatch(const unsigned int forwardLookId)
{
  DebugMessage("DisplayTargetPatch()");
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display target patch for iterations > 0." << std::endl;
    return;
    }

  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }

  FloatVectorImageType::Pointer currentImage = this->IntermediateImages[this->IterationToDisplay - 1].Image;

  const CandidatePairs& candidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1][forwardLookId];

  // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration (as the current mask has been cleared where the target patch was copied).
  Mask::Pointer currentMask = this->IntermediateImages[this->IterationToDisplay - 1].MaskImage;

  // Target
  QImage targetImage = Helpers::GetQImageColor<FloatVectorImageType>(currentImage, candidatePairs.TargetPatch.Region);

  targetImage = Helpers::FitToGraphicsView(targetImage, gfxTarget);
  this->TargetPatchScene->addPixmap(QPixmap::fromImage(targetImage));

  //Refresh();
}

void Form::DisplayResultPatch(const unsigned int forwardLookId, const unsigned int topPatchId)
{
  DebugMessage("DisplayResultPatch()");
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display result patch for iterations > 0." << std::endl;
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }

  FloatVectorImageType::Pointer currentImage = this->IntermediateImages[this->IterationToDisplay - 1].Image;
  
  const CandidatePairs& candidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1][forwardLookId];

  const PatchPair& patchPair = candidatePairs[topPatchId];
  
  // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration (as the current mask has been cleared where the target patch was copied).
  Mask::Pointer currentMask = this->IntermediateImages[this->IterationToDisplay - 1].MaskImage;

  itk::Size<2> regionSize = patchPair.SourcePatch.Region.GetSize(); // this could equally as well be TargetPatch
  
  QImage qimage(regionSize[0], regionSize[1], QImage::Format_RGB888);

  itk::ImageRegionIterator<FloatVectorImageType> sourceIterator(currentImage, patchPair.SourcePatch.Region);
  itk::ImageRegionIterator<FloatVectorImageType> targetIterator(currentImage, patchPair.TargetPatch.Region);
  itk::ImageRegionIterator<Mask> maskIterator(currentMask, patchPair.TargetPatch.Region);
  
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
    QColor pixelColor(static_cast<int>(pixel[0]), static_cast<int>(pixel[1]), static_cast<int>(pixel[2]));
    qimage.setPixel(offset[0], offset[1], pixelColor.rgb());

    ++sourceIterator;
    ++targetIterator;
    ++maskIterator;
    }

  qimage.setPixel(regionSize[0]/2, regionSize[1]/2, this->CenterPixelColor.rgb());
  
  // Flip the image
  qimage = qimage.mirrored(false, true);
  
  qimage = Helpers::FitToGraphicsView(qimage, gfxResult);
  this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));

  //Refresh();
}

void Form::DisplayUsedPatches()
{
  DebugMessage("DisplayUsedPatches()");

  // There are no patches used in the 0th iteration (initial conditions) so it doesn't make sense to display them.
  // Instead we display blank images.
  if(this->IterationToDisplay < 1)
    {
    //QImage blankImage;
    //blankImage = Helpers::FitToGraphicsView(blankImage, gfxTarget);

    //this->TargetPatchScene->addPixmap(QPixmap::fromImage(blankImage));
    //this->SourcePatchScene->addPixmap(QPixmap::fromImage(blankImage));

    this->TargetPatchScene->clear();
    this->SourcePatchScene->clear();

    return;
    }
    
  DisplaySourcePatch(0, 0);
  DisplayTargetPatch(0);
  DisplayResultPatch(0,0);
  Refresh();
}

void Form::HighlightForwardLookPatches()
{
  DebugMessage("HighlightForwardLookPatches()");

  // Delete any current highlight patches. We want to delete these (if they exist) no matter what because then they won't be displayed if the box is not checked (they will respect the check box).
  Helpers::BlankImage(this->AllForwardLookOutlinesLayer.ImageData);

  // If the user has not requested to display the patches, quit.
  if(!this->chkDisplayForwardLookPatchLocations->isChecked())
    {
    return;
    }

  if(!this->IterationToDisplay < 1)
    {
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }
    
  // Get the candidate patches and make sure we have requested a valid set.
  const std::vector<CandidatePairs>& candidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1];

  unsigned char borderColor[3];
  Helpers::QColorToUCharColor(this->AllForwardLookPatchColor, borderColor);
  unsigned char centerPixelColor[3];
  Helpers::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);
  
  for(unsigned int candidateId = 0; candidateId < candidatePairs.size(); ++candidateId)
    {
    const Patch& currentPatch = candidatePairs[candidateId].TargetPatch;
    std::cout << "Outlining " << currentPatch.Region << std::endl;
    //DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);
    
    Helpers::BlankAndOutlineRegion(this->AllForwardLookOutlinesLayer.ImageData, currentPatch.Region, borderColor);
    
    Helpers::SetRegionCenterPixel(this->AllForwardLookOutlinesLayer.ImageData, currentPatch.Region, centerPixelColor);
    }

  if(this->forwardLookingTableWidget->currentRow() < 0)
    {
    this->forwardLookingTableWidget->selectRow(0);
    }
  HighlightSelectedForwardLookPatch(this->forwardLookingTableWidget->currentRow());

  this->qvtkWidget->GetRenderWindow()->Render();
}


void Form::HighlightSourcePatches()
{

  DebugMessage("HighlightSourcePatches()");

  // Delete any current highlight patches. We want to delete these (if they exist) no matter what because then they won't be displayed if the box is not checked (they will respect the check box).
  Helpers::BlankImage(this->AllSourcePatchOutlinesLayer.ImageData);

  if(!this->chkDisplaySourcePatchLocations->isChecked())
    {
    return;
    }

  if(this->IterationToDisplay < 1)
    {
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }
//   CandidatePairs& candidatePairs;
//   bool valid = this->Inpainting.GetPotentialCandidatePairs(this->IterationToDisplay - 1, this->forwardLookingTableWidget->currentRow(), candidatePairs);
//   if(!valid)
//     {
//     return;
//     }

  const CandidatePairs& candidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1][this->forwardLookingTableWidget->currentRow()];

  unsigned char borderColor[3];
  Helpers::QColorToUCharColor(this->AllSourcePatchColor, borderColor);
  unsigned char centerPixelColor[3];
  Helpers::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);

  unsigned int numberToDisplay = std::min(candidatePairs.size(), this->txtNumberOfTopPatches->text().toUInt());
  for(unsigned int candidateId = 0; candidateId < numberToDisplay; ++candidateId)
    {
    //DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);
    const Patch& currentPatch = candidatePairs[candidateId].SourcePatch;
    Helpers::BlankAndOutlineRegion(this->AllSourcePatchOutlinesLayer.ImageData, currentPatch.Region, borderColor);
    Helpers::SetRegionCenterPixel(this->AllSourcePatchOutlinesLayer.ImageData, currentPatch.Region, centerPixelColor);
    }

  //std::cout << "Selected forward look patch should be: " << this->forwardLookingTableWidget->currentRow() << std::endl;
  if(this->topPatchesTableWidget->currentRow() < 0)
    {
    this->topPatchesTableWidget->selectRow(0);
    }

  int idColumnId = GetColumnIdByHeader("Id");
  unsigned int sourcePatchId = topPatchesTableWidget->item(this->topPatchesTableWidget->currentRow(), idColumnId)->data(Qt::DisplayRole).toUInt();
  HighlightSelectedSourcePatch(sourcePatchId);

  this->qvtkWidget->GetRenderWindow()->Render();

}

void Form::HighlightUsedPatches()
{
  DebugMessage("HighlightUsedPatches()");
  
  //unsigned int patchSize = Helpers::SideLengthFromRadius(this->txtPatchRadius->text().toUInt());
  //DebugMessage<unsigned int>("Patch size: ", patchSize);

  PatchPair patchPair;
  bool pairValid = false;
  pairValid = this->Inpainting.GetUsedPatchPair(this->IterationToDisplay - 1, patchPair);
  if(!pairValid)
    {
    std::cerr << "You have requested an invalid pair!" << std::endl;
    return;
    }

  unsigned char centerPixelColor[3];
  Helpers::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);
  
  // Target
  Patch targetPatch = patchPair.TargetPatch;

  unsigned int patchSize = targetPatch.Region.GetSize()[0];
  //std::cout << "Displaying used target patch " << this->CurrentUsedPatchDisplayed << " : " << targetPatch.Region << std::endl;
  DebugMessage<itk::ImageRegion<2> >("Target patch region: ", targetPatch.Region);
  this->UsedTargetPatchLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
  unsigned char targetPatchColor[3];
  Helpers::QColorToUCharColor(this->UsedTargetPatchColor, targetPatchColor);
  Helpers::BlankAndOutlineImage(this->UsedTargetPatchLayer.ImageData, targetPatchColor);
  Helpers::SetImageCenterPixel(this->UsedTargetPatchLayer.ImageData, centerPixelColor);
  this->UsedTargetPatchLayer.ImageSlice->SetPosition(targetPatch.Region.GetIndex()[0], targetPatch.Region.GetIndex()[1], 0);

  // Source
  Patch sourcePatch = patchPair.SourcePatch;

  //std::cout << "Displaying used source patch " << this->CurrentUsedPatchDisplayed << " : " << sourcePatch.Region << std::endl;
  DebugMessage<itk::ImageRegion<2> >("Source patch region: ", sourcePatch.Region);
  this->UsedSourcePatchLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
  unsigned char sourcePatchColor[3];
  Helpers::QColorToUCharColor(this->UsedSourcePatchColor, sourcePatchColor);
  Helpers::BlankAndOutlineImage(this->UsedSourcePatchLayer.ImageData, sourcePatchColor);
  Helpers::SetImageCenterPixel(this->UsedSourcePatchLayer.ImageData, centerPixelColor);
  this->UsedSourcePatchLayer.ImageSlice->SetPosition(sourcePatch.Region.GetIndex()[0], sourcePatch.Region.GetIndex()[1], 0);

  Refresh();

}

void Form::DisplayUsedPatchInformation()
{
  DebugMessage("DisplayUsedPatchInformation()");
  
  SetupForwardLookingTable();
  
  // There is a -1 offset here because the 0th used pair corresponds to the pair after iteration 1 because there are no used patches after iteration 0 (initial conditions)
  PatchPair patchPair;
  bool validPair = this->Inpainting.GetUsedPatchPair(this->IterationToDisplay - 1, patchPair);
  if(!validPair)
    {
    std::cerr << "You have requested an invalid pair!" << std::endl;
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

void Form::CreatePotentialTargetPatchesImage()
{
  DebugMessage("CreatePotentialTargetPatchesImage()");
  // Draw potential patch pairs
  
//   std::stringstream ssPatchPairsFile;
//   ssPatchPairsFile << "Debug/PatchPairs_" << Helpers::ZeroPad(this->Inpainting.GetIteration(), 3) << ".txt";
//   OutputPairs(potentialPatchPairs, ssPatchPairsFile.str());

  if(this->IterationToDisplay < 1)
    {
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }
    
  this->PotentialTargetPatchesImage->SetRegions(this->Inpainting.GetFullRegion());
  this->PotentialTargetPatchesImage->Allocate();
  this->PotentialTargetPatchesImage->FillBuffer(0);

  const std::vector<CandidatePairs>& potentialCandidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1];
  
  for(unsigned int i = 0; i < potentialCandidatePairs.size(); ++i)
    {
    Helpers::BlankAndOutlineRegion<UnsignedCharScalarImageType>(this->PotentialTargetPatchesImage,
                                                                potentialCandidatePairs[i].TargetPatch.Region, static_cast<unsigned char>(0),
                                                                static_cast<unsigned char>(255));
    }

  vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
  Helpers::ITKScalarImageToScaledVTKImage<UnsignedCharScalarImageType>(this->PotentialTargetPatchesImage, temp);
  Helpers::MakePixelsTransparent(temp, this->PotentialPatchesLayer.ImageData, 0);

  Refresh();
}

void Form::OutputPairs(const std::vector<PatchPair>& patchPairs, const std::string& filename)
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

void Form::ChangeDisplayedIteration()
{
  // This should be called only when the iteration actually changed.
  
  DebugMessage("ChangeDisplayedIteration()");

  std::stringstream ss;
  ss << this->IterationToDisplay;
  this->lblCurrentIteration->setText(ss.str().c_str());

  if(this->IterationToDisplay > 0)
    {
    DisplayUsedPatches();
    CreatePotentialTargetPatchesImage();
    HighlightUsedPatches();
    DisplayUsedPatchInformation();
    }
  else
    {
    this->forwardLookingTableWidget->setRowCount(0);
    this->topPatchesTableWidget->setRowCount(0);
    this->TargetPatchScene->clear();
    this->SourcePatchScene->clear();
    this->ResultPatchScene->clear();
    }

  Refresh();
}

void Form::SetupInitialIntermediateImages()
{
  DebugMessage("SetupInitialIntermediateImages()");
  
  InpaintingVisualizationStack stack;
  
  //Helpers::DeepCopyVectorImage<FloatVectorImageType>(this->UserImage, stack.Image);
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), stack.Image);
  Helpers::DeepCopy<Mask>(this->UserMaskImage, stack.MaskImage);
  Helpers::DeepCopy<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), stack.Boundary);
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetPriorityImage(), stack.Priority);
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetDataImage(), stack.Data);
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetConfidenceImage(), stack.Confidence);
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetConfidenceMapImage(), stack.ConfidenceMap);
  Helpers::DeepCopy<FloatVector2ImageType>(this->Inpainting.GetBoundaryNormalsImage(), stack.BoundaryNormals);
  Helpers::DeepCopy<FloatVector2ImageType>(this->Inpainting.GetIsophoteImage(), stack.Isophotes);
  //Helpers::DeepCopy<UnsignedCharScalarImageType>(this->PotentialTargetPatchesImage, stack.PotentialTargetPatchesImage);

  this->IntermediateImages.clear();
  this->IntermediateImages.push_back(stack);
  
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::IterationComplete()
{
  std::cout << "IterationComplete()" << std::endl;
  //DebugMessage("Enter IterationComplete()");

  // Save the intermediate images
  
  InpaintingVisualizationStack stack;
  
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), stack.Image);
  Helpers::DeepCopy<Mask>(this->Inpainting.GetMaskImage(), stack.MaskImage);
  Helpers::DeepCopy<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), stack.Boundary);
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetPriorityImage(), stack.Priority);
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetDataImage(), stack.Data);
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetConfidenceImage(), stack.Confidence);
  Helpers::DeepCopy<FloatScalarImageType>(this->Inpainting.GetConfidenceMapImage(), stack.ConfidenceMap);
  Helpers::DeepCopy<FloatVector2ImageType>(this->Inpainting.GetBoundaryNormalsImage(), stack.BoundaryNormals);
  Helpers::DeepCopy<FloatVector2ImageType>(this->Inpainting.GetIsophoteImage(), stack.Isophotes);
  Helpers::DeepCopy<UnsignedCharScalarImageType>(this->PotentialTargetPatchesImage, stack.PotentialTargetPatchesImage);

  this->IntermediateImages.push_back(stack);

  if(this->chkRecordSteps->isChecked())
    {
    this->AllPotentialCandidatePairs.push_back(this->Inpainting.GetPotentialCandidatePairs());
    this->Recorded.push_back(true);
    }
  else
    {
    this->Recorded.push_back(false);
    }
  
  // After one iteration, GetNumberOfCompletedIterations will be 1. This is exactly the set of intermediate images we want to display,
  // because the 0th intermediate images are the original inputs.
  
  if(this->chkLive->isChecked())
    {
    this->IterationToDisplay = this->Inpainting.GetNumberOfCompletedIterations();
    }

  ChangeDisplayedIteration();

  Refresh();
  
  DebugMessage("Leave IterationComplete()");
}

void Form::IterationCompleteSlot()
{
  DebugMessage("IterationCompleteSlot()");
  IterationComplete();
}

void Form::SetupForwardLookingTable()
{
  DebugMessage("SetupForwardLookingTable()");
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display result patch for iterations > 0." << std::endl;
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }
  
  // Clear the table
  this->forwardLookingTableWidget->setRowCount(0);

  // How big to display the patch.
  unsigned int patchDisplaySize = 100;

  // If the patch is exactly as big as the cell, we cannot tell that the cell/row is selected/highlighted.
  // To fix this, we make the cell a little bit bigger than the patch.
  unsigned int cellSize = patchDisplaySize + 10;
  
  this->forwardLookingTableWidget->setColumnWidth(0, cellSize);

  // There is a -1 offset here because the 0th patch pair to be stored is after iteration 1 (as after the 0th iteration (initial conditions) there are no used patch pairs)
  const std::vector<CandidatePairs>& allCandidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1];
  
  unsigned int numberToDisplay = std::min(allCandidatePairs.size(), this->txtNumberOfForwardLook->text().toUInt());
  if(allCandidatePairs.size() != this->txtNumberOfForwardLook->text().toUInt())
    {
    std::cerr << "Warning: Number of pairs (" << allCandidatePairs.size() << ") does not match requested number ("
              << this->txtNumberOfForwardLook->text().toUInt() << ")" << std::endl;
    }

  //for(unsigned int forwardLookId = 0; forwardLookId < candidatePairs.size(); ++forwardLookId)
  for(unsigned int forwardLookId = 0; forwardLookId < numberToDisplay; ++forwardLookId)
    {
    this->forwardLookingTableWidget->insertRow(this->forwardLookingTableWidget->rowCount());
    this->forwardLookingTableWidget->setRowHeight(forwardLookId, cellSize);
  
    const Patch& currentForwardLookPatch = allCandidatePairs[forwardLookId].TargetPatch;

    // The -1 offset here is so that the patch that was actually used is still masked in the table. (If we use the current intermediate image, one of the patches would have already been filled).
    QImage patchImage = Helpers::GetQImageColor<FloatVectorImageType>(this->IntermediateImages[this->IterationToDisplay - 1].Image, currentForwardLookPatch.Region);
    // This does exactly the same thing, but explicitly masks the hole pixels. We would rather do the above so that we ensure everything is synchronized.
    //QImage patchImage = Helpers::GetQImageMasked<FloatVectorImageType>(this->IntermediateImages[this->IterationToDisplay - 1].Image,
    //                                                                   this->IntermediateImages[this->IterationToDisplay - 1].MaskImage,
    //                                                                   currentForwardLookPatch.Region);
    patchImage = patchImage.scaledToHeight(patchDisplaySize);
  
    // Create a label with the image as the way to display an image in the table.
    QLabel* imageLabel = new QLabel;
    imageLabel->setPixmap(QPixmap::fromImage(patchImage));
    imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    
    this->forwardLookingTableWidget->setCellWidget(forwardLookId, 0, imageLabel); // (row, col, widget)

    // Display priority in the table
    QTableWidgetItem* priorityLabel = new QTableWidgetItem;
    priorityLabel->setData(Qt::DisplayRole, allCandidatePairs[forwardLookId].Priority);
    this->forwardLookingTableWidget->setItem(forwardLookId, 1, priorityLabel); // (row, col, widget)

    // Display priority in the table
    QTableWidgetItem* locationLabel = new QTableWidgetItem;
    std::stringstream ssLocation;
    ssLocation << "(" << currentForwardLookPatch.Region.GetIndex()[0] << ", " << currentForwardLookPatch.Region.GetIndex()[1] << ")";
    locationLabel->setText(ssLocation.str().c_str());
    this->forwardLookingTableWidget->setItem(forwardLookId, 2, locationLabel); // (row, col, widget)
    }
    
  //this->forwardLookingTableWidget->resizeRowsToContents();
  this->forwardLookingTableWidget->resizeColumnsToContents();

  this->forwardLookingTableWidget->selectRow(0);
  
  // Always display the corresponding source patches for the 0th forward look patch until the user clicks a different row
  SetupTopPatchesTable(0);
  HighlightSelectedForwardLookPatch(0);
}

void Form::SetupTopPatchesTable(unsigned int forwardLookId)
{
  DebugMessage("SetupTopPatchesTable()");
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display result patch for iterations > 0." << std::endl;
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }

  // Clear the table
  this->topPatchesTableWidget->setRowCount(0);

  unsigned int patchDisplaySize = 100;
  this->topPatchesTableWidget->setColumnWidth(0, patchDisplaySize);

  // There is a -1 offset here because the 0th patch pair to be stored is after iteration 1 (as after the 0th iteration (initial conditions) there are no used patch pairs)
  const CandidatePairs& candidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1][forwardLookId];

  unsigned int numberToDisplay = std::min(candidatePairs.size(), this->txtNumberOfTopPatches->text().toUInt());
  if(candidatePairs.size() != this->txtNumberOfTopPatches->text().toUInt())
    {
    std::cerr << "Warning: Number of pairs (" << candidatePairs.size() << ") does not match requested number (" << this->txtNumberOfTopPatches->text().toUInt() << ")" << std::endl;
    }
  //for(unsigned int pairId = 0; pairId < candidatePairs.size(); ++pairId)
  for(unsigned int pairId = 0; pairId < numberToDisplay; ++pairId)
    {
    this->topPatchesTableWidget->insertRow(this->topPatchesTableWidget->rowCount());
    this->topPatchesTableWidget->setRowHeight(pairId, patchDisplaySize);

    const Patch& currentSourcePatch = candidatePairs[pairId].SourcePatch;

    QImage sourceImage = Helpers::GetQImageColor<FloatVectorImageType>(this->IntermediateImages[this->IterationToDisplay].Image, currentSourcePatch.Region);
    sourceImage = sourceImage.scaledToHeight(patchDisplaySize);

    // Create a label with the image as the way to display an image in the table.
    QLabel* imageLabel = new QLabel;
    imageLabel->setPixmap(QPixmap::fromImage(sourceImage));
    imageLabel->setScaledContents(true);
    this->topPatchesTableWidget->setCellWidget(pairId, 0, imageLabel);


    // Display patch match scores
    QTableWidgetItem* totalAbsoluteDifferenceLabel = new QTableWidgetItem;
    totalAbsoluteDifferenceLabel->setData(Qt::DisplayRole, candidatePairs[pairId].GetTotalAbsoluteDifference());
    this->topPatchesTableWidget->setItem(pairId, 1, totalAbsoluteDifferenceLabel);
    
    QTableWidgetItem* totalSquaredDifferenceLabel = new QTableWidgetItem;
    totalSquaredDifferenceLabel->setData(Qt::DisplayRole, candidatePairs[pairId].GetTotalSquaredDifference());
    this->topPatchesTableWidget->setItem(pairId, 2, totalSquaredDifferenceLabel);
    
    QTableWidgetItem* averageAbsoluteDifferenceLabel = new QTableWidgetItem;
    averageAbsoluteDifferenceLabel->setData(Qt::DisplayRole, candidatePairs[pairId].GetAverageAbsoluteDifference());
    this->topPatchesTableWidget->setItem(pairId, 3, averageAbsoluteDifferenceLabel);
    
    QTableWidgetItem* averageSquaredDifferenceLabel = new QTableWidgetItem;
    averageSquaredDifferenceLabel->setData(Qt::DisplayRole, candidatePairs[pairId].GetAverageSquaredDifference());
    this->topPatchesTableWidget->setItem(pairId, 4, averageSquaredDifferenceLabel);

    // Display boundary pixel difference score
    QTableWidgetItem* boundaryPixelDifferenceLabel = new QTableWidgetItem;
    boundaryPixelDifferenceLabel->setData(Qt::DisplayRole, candidatePairs[pairId].GetBoundaryPixelDifference());
    this->topPatchesTableWidget->setItem(pairId, 5, boundaryPixelDifferenceLabel);
    
    // Display boundary isophote difference score
    QTableWidgetItem* boundaryIsophoteAngleDifferenceLabel = new QTableWidgetItem;
    boundaryIsophoteAngleDifferenceLabel->setData(Qt::DisplayRole, candidatePairs[pairId].GetBoundaryIsophoteAngleDifference());
    this->topPatchesTableWidget->setItem(pairId, 6, boundaryIsophoteAngleDifferenceLabel);

    QTableWidgetItem* boundaryIsophoteStrengthDifferenceLabel = new QTableWidgetItem;
    boundaryIsophoteStrengthDifferenceLabel->setData(Qt::DisplayRole, candidatePairs[pairId].GetBoundaryIsophoteStrengthDifference());
    this->topPatchesTableWidget->setItem(pairId, 7, boundaryIsophoteStrengthDifferenceLabel);
    
    // Display total score
    QTableWidgetItem* totalScoreLabel = new QTableWidgetItem;
    totalScoreLabel->setData(Qt::DisplayRole, candidatePairs[pairId].GetTotalScore());
    this->topPatchesTableWidget->setItem(pairId, 8, totalScoreLabel);
    
    // Store the patch/row Id. This is needed in case we sort the table using the widget header buttons.
    QTableWidgetItem* idLabel = new QTableWidgetItem;
    idLabel->setData(Qt::DisplayRole, pairId);
    this->topPatchesTableWidget->setItem(pairId, 9, idLabel);
    this->topPatchesTableWidget->setColumnHidden(9, true); // We don't want to display this value, just track it.
    
    // Display patch location
    std::stringstream ssLocation;
    ssLocation << "( " << currentSourcePatch.Region.GetIndex()[0] << ", " << currentSourcePatch.Region.GetIndex()[1] << ")";

    QTableWidgetItem* locationLabel = new QTableWidgetItem;
    locationLabel->setText(ssLocation.str().c_str());
    this->topPatchesTableWidget->setItem(pairId, 10, locationLabel);
    }

  this->topPatchesTableWidget->selectRow(0);
  //this->topPatchesTableWidget->horizontalHeader()->resizeColumnsToContents();
  this->topPatchesTableWidget->resizeColumnsToContents();
}

void Form::HighlightSelectedForwardLookPatch(const unsigned int id)
{
  DebugMessage("HighlightSelectedForwardLookPatch()");
  // Highlight the selected forward look patch in a different color than the rest if the user has chosen to display forward look patch locations.
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display result patch for iterations > 0." << std::endl;
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }
    
  if(!this->chkDisplayForwardLookPatchLocations->isChecked())
    {
    return;
    }
  else
    {
    unsigned int patchRadius = this->txtPatchRadius->text().toUInt();
    unsigned int patchSize = Helpers::SideLengthFromRadius(patchRadius);

    this->SelectedForwardLookOutlineLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
    this->SelectedForwardLookOutlineLayer.ImageData->AllocateScalars();
    unsigned char selectedPatchColor[3];
    Helpers::QColorToUCharColor(this->SelectedForwardLookPatchColor, selectedPatchColor);
    Helpers::BlankAndOutlineImage(this->SelectedForwardLookOutlineLayer.ImageData, selectedPatchColor);
    unsigned char centerPixelColor[3];
    Helpers::QColorToUCharColor(this->CenterPixelColor, centerPixelColor);
    Helpers::SetImageCenterPixel(this->SelectedForwardLookOutlineLayer.ImageData, centerPixelColor);

    // There is a -1 offset here because the 0th patch pair to be stored is after iteration 1 (as after the 0th iteration (initial conditions) there are no used patch pairs)
    std::vector<CandidatePairs>& allCandidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1];

    const Patch& selectedPatch = allCandidatePairs[id].TargetPatch;
    this->SelectedForwardLookOutlineLayer.ImageSlice->SetPosition(selectedPatch.Region.GetIndex()[0], selectedPatch.Region.GetIndex()[1], 0);

    this->qvtkWidget->GetRenderWindow()->Render();
    }

}


void Form::HighlightSelectedSourcePatch(const unsigned int id)
{
  DebugMessage("HighlightSelectedSourcePatch()");
  // Highlight the selected source patch in a different color than the rest if the user has chosen to display forward look patch locations.
  
  if(this->IterationToDisplay < 1)
    {
    std::cerr << "Can only display result patch for iterations > 0." << std::endl;
    return;
    }
    
  if(!this->Recorded[this->IterationToDisplay - 1])
    {
    return;
    }

  if(!this->chkDisplaySourcePatchLocations->isChecked())
    {
    return;
    }
  else
    {
    unsigned int patchRadius = this->txtPatchRadius->text().toUInt();
    unsigned int patchSize = Helpers::SideLengthFromRadius(patchRadius);

    this->SelectedSourcePatchOutlineLayer.ImageData->SetDimensions(patchSize, patchSize, 1);
    this->SelectedSourcePatchOutlineLayer.ImageData->AllocateScalars();
    unsigned char selectedPatchColor[3];
    Helpers::QColorToUCharColor(this->SelectedSourcePatchColor, selectedPatchColor);
    Helpers::BlankAndOutlineImage(this->SelectedSourcePatchOutlineLayer.ImageData, selectedPatchColor);
    unsigned char centerPixelColor[3] = {122, 0, 255};
    Helpers::SetImageCenterPixel(this->SelectedSourcePatchOutlineLayer.ImageData, centerPixelColor);

    // There is a -1 offset here because the 0th patch pair to be stored is after iteration 1 (as after the 0th iteration (initial conditions) there are no used patch pairs)
    const CandidatePairs& candidatePairs = this->AllPotentialCandidatePairs[this->IterationToDisplay - 1][this->forwardLookingTableWidget->currentRow()];

    const Patch& selectedPatch = candidatePairs[id].SourcePatch;
    this->SelectedSourcePatchOutlineLayer.ImageSlice->SetPosition(selectedPatch.Region.GetIndex()[0], selectedPatch.Region.GetIndex()[1], 0);

    this->qvtkWidget->GetRenderWindow()->Render();
    }
}

void Form::on_forwardLookingTableWidget_cellClicked(int row, int col)
{
  //std::cout << "Clicked row " << row << std::endl;

  // Setup the top source patches table for this forward look patch.
  SetupTopPatchesTable(row);

  // Display the big target patch
  DisplayTargetPatch(row);
  DisplayResultPatch(row, 0);

  HighlightSelectedForwardLookPatch(row);
  HighlightSourcePatches();
}

void Form::on_topPatchesTableWidget_cellClicked(int row, int col)
{
  // Get the column number of the "Id" column.
  int idColumnId = GetColumnIdByHeader("Id");
  if(idColumnId < 0)
    {
    std::cerr << "Requested invalid column!" << std::endl;
    return;
    }

  //unsigned int sourcePatchId = topPatchesTableWidget->item(row, idColumnId)->text().toUInt(); // This also works
  unsigned int sourcePatchId = topPatchesTableWidget->item(row, idColumnId)->data(Qt::DisplayRole).toUInt();

  // Here we should update the big displayed source patch
  DisplaySourcePatch(forwardLookingTableWidget->currentRow(), sourcePatchId);
  DisplayResultPatch(forwardLookingTableWidget->currentRow(), sourcePatchId);

  HighlightSourcePatches();
  HighlightSelectedSourcePatch(sourcePatchId);
}

int Form::GetColumnIdByHeader(const std::string& header)
{
  //std::cout << "There are " << static_cast<unsigned int>(topPatchesTableWidget->columnCount()) << " columns" << std::endl;
  //std::cout << "Looking for column with header = " << header << std::endl;
  
  for(unsigned int i = 0; i < static_cast<unsigned int>(topPatchesTableWidget->columnCount()); ++i)
    {
    if(topPatchesTableWidget->horizontalHeaderItem(i)->text().toStdString().compare(header) == 0)
      {
      return i;
      }
    }
  return -1;
}
