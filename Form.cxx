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
#include "Helpers.h"
#include "InteractorStyleImageNoLevel.h"
#include "Mask.h"
#include "Types.h"

const unsigned char Form::Green[3] = {0,255,0};
const unsigned char Form::Red[3] = {255,0,0};


void Form::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();
  
  help->setReadOnly(true);
  help->append("<h1>Criminisi Inpainting</h1>\
  Load and image and a mask. <br/>\
  Set the settings such as patch size. <br/>\
  Click Inpaint.<br/> <p/>"
  );
  help->show();
}

// Constructor
Form::Form()
{
  this->setupUi(this);

  this->CurrentUsedPatchDisplayed = -1;
  
  this->DebugImages = false;
  this->DebugMessages = false;

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");
  QIcon saveIcon = QIcon::fromTheme("document-save");
  
  // Setup toolbar
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  actionOpenMask->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMask);
  actionOpenMask->setEnabled(false);

  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);

  this->Flipped = false;
  
  //this->InteractorStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageNoLevel>::New();
  
  // Initialize and link the image display objects
  this->VTKImage = vtkSmartPointer<vtkImageData>::New();
  this->ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->ImageSliceMapper->BorderOn();
  
  this->ImageSliceMapper->SetInputConnection(this->VTKImage->GetProducerPort());
  this->ImageSlice->SetMapper(this->ImageSliceMapper);
  
  // Source patch display
  this->SourcePatch = vtkSmartPointer<vtkImageData>::New();
  this->SourcePatchSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->SourcePatchSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->SourcePatchSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->SourcePatchSliceMapper->BorderOn();
  
  this->SourcePatchSliceMapper->SetInputConnection(this->SourcePatch->GetProducerPort());
  this->SourcePatchSlice->SetMapper(this->SourcePatchSliceMapper);
  
  // Target patch display
  this->TargetPatch = vtkSmartPointer<vtkImageData>::New();
  this->TargetPatchSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->TargetPatchSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->TargetPatchSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->TargetPatchSliceMapper->BorderOn();
  
  this->TargetPatchSliceMapper->SetInputConnection(this->TargetPatch->GetProducerPort());
  this->TargetPatchSlice->SetMapper(this->TargetPatchSliceMapper);
  
  // Initialize and link the priority image display objects
  this->VTKPriorityImage = vtkSmartPointer<vtkImageData>::New();
  this->PriorityImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->PriorityImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->PriorityImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->PriorityImageSliceMapper->BorderOn();
  
  this->PriorityImageSliceMapper->SetInputConnection(this->VTKPriorityImage->GetProducerPort());
  this->PriorityImageSlice->SetMapper(this->PriorityImageSliceMapper);
    
  // Initialize and link the confidence image display objects
  this->VTKConfidenceImage = vtkSmartPointer<vtkImageData>::New();
  this->ConfidenceImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ConfidenceImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->ConfidenceImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->ConfidenceImageSliceMapper->BorderOn();
  
  this->ConfidenceImageSliceMapper->SetInputConnection(this->VTKConfidenceImage->GetProducerPort());
  this->ConfidenceImageSlice->SetMapper(this->ConfidenceImageSliceMapper);
  
  // Initialize and link the boundary image display objects
  this->VTKBoundaryImage = vtkSmartPointer<vtkImageData>::New();
  this->BoundaryImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->BoundaryImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->BoundaryImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->BoundaryImageSliceMapper->BorderOn();
  
  this->BoundaryImageSliceMapper->SetInputConnection(this->VTKBoundaryImage->GetProducerPort());
  this->BoundaryImageSlice->SetMapper(this->BoundaryImageSliceMapper);

  // Initialize and link the mask image display objects
  this->VTKMaskImage = vtkSmartPointer<vtkImageData>::New();
  this->MaskImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->MaskImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->MaskImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->MaskImageSliceMapper->BorderOn();
  
  this->MaskImageSliceMapper->SetInputConnection(this->VTKMaskImage->GetProducerPort());
  this->MaskImageSlice->SetMapper(this->MaskImageSliceMapper);
  
  // Initialize and link the data image display objects
  this->VTKDataImage = vtkSmartPointer<vtkImageData>::New();
  this->DataImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->DataImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->DataImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();

  this->DataImageSliceMapper->SetInputConnection(this->VTKDataImage->GetProducerPort());
  this->DataImageSlice->SetMapper(this->DataImageSliceMapper);

  // Setup the arrows for boundary normals and isophote visualization
  vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New();
  arrowSource->Update();
  
  // Isophote display
  this->IsophoteMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->VTKIsophoteImage = vtkSmartPointer<vtkImageData>::New();
  this->IsophoteActor = vtkSmartPointer<vtkActor>::New();
  this->IsophoteGlyph = vtkSmartPointer<vtkGlyph2D>::New();
  this->VTKNonZeroIsophoteVectors = vtkSmartPointer<vtkPolyData>::New();
  
  this->IsophoteGlyph->SetInputConnection(this->VTKNonZeroIsophoteVectors->GetProducerPort());
  this->IsophoteGlyph->SetSource(arrowSource->GetOutput());
  this->IsophoteGlyph->OrientOn();
  this->IsophoteGlyph->SetVectorModeToUseVector();
  this->IsophoteGlyph->SetScaleFactor(10);
  
  this->IsophoteMapper->SetInputConnection(this->IsophoteGlyph->GetOutputPort());
  this->IsophoteActor->SetMapper(this->IsophoteMapper);
  
  // BoundaryNormals display
  this->BoundaryNormalsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->VTKBoundaryNormalsImage = vtkSmartPointer<vtkImageData>::New();
  this->BoundaryNormalsActor = vtkSmartPointer<vtkActor>::New();
  this->BoundaryNormalsGlyph = vtkSmartPointer<vtkGlyph2D>::New();
  this->VTKNonZeroBoundaryNormals = vtkSmartPointer<vtkPolyData>::New();
  
  this->BoundaryNormalsGlyph->SetInputConnection(this->VTKNonZeroBoundaryNormals->GetProducerPort());
  this->BoundaryNormalsGlyph->SetSource(arrowSource->GetOutput());
  this->BoundaryNormalsGlyph->OrientOn();
  this->BoundaryNormalsGlyph->SetVectorModeToUseVector();
  this->BoundaryNormalsGlyph->SetScaleFactor(10);
  
  this->BoundaryNormalsMapper->SetInputConnection(this->BoundaryNormalsGlyph->GetOutputPort());
  this->BoundaryNormalsActor->SetMapper(this->BoundaryNormalsMapper);
  
  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);
  
  this->Renderer->AddViewProp(this->ImageSlice);
  this->Renderer->AddViewProp(this->ConfidenceImageSlice);
  this->Renderer->AddViewProp(this->BoundaryImageSlice);
  this->Renderer->AddViewProp(this->PriorityImageSlice);
  this->Renderer->AddViewProp(this->DataImageSlice);
  this->Renderer->AddViewProp(this->IsophoteActor);
  this->Renderer->AddViewProp(this->BoundaryNormalsActor);
  this->Renderer->AddViewProp(this->MaskImageSlice);
  this->Renderer->AddViewProp(this->TargetPatchSlice);
  this->Renderer->AddViewProp(this->SourcePatchSlice);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  
  //this->Image = NULL;
  //this->Mask = NULL;
  
  this->Image = FloatVectorImageType::New();
  this->MaskImage = Mask::New();
  
  connect(&ComputationThread, SIGNAL(StartProgressSignal()), this, SLOT(StartProgressSlot()), Qt::QueuedConnection);
  connect(&ComputationThread, SIGNAL(StopProgressSignal()), this, SLOT(StopProgressSlot()), Qt::QueuedConnection);
  connect(&ComputationThread, SIGNAL(IterationCompleteSignal()), this, SLOT(IterationCompleteSlot()), Qt::QueuedConnection);

  // Set the progress bar to marquee mode
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();
  
  ComputationThread.SetObject(&(this->Inpainting));
  
  connect(&ComputationThread, SIGNAL(RefreshSignal()), this, SLOT(RefreshSlot()), Qt::QueuedConnection);
};
  
void Form::on_chkDebugImages_clicked()
{
  QDir directoryMaker;
  directoryMaker.mkdir("Debug");
  
  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->DebugImages = this->chkDebugImages->isChecked();
  std::cout << "this->DebugImages: " << this->DebugImages << std::endl;
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

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }
    
  Helpers::WriteImage<FloatVectorImageType>(this->Inpainting.GetResult(), fileName.toStdString());
  
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

void Form::on_actionOpenImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  // Set the working directory
  QFileInfo fileInfo(fileName);
  std::string workingDirectory = fileInfo.absoluteDir().absolutePath().toStdString() + "/";
  std::cout << "Working directory set to: " << workingDirectory << std::endl;
  QDir::setCurrent(QString(workingDirectory.c_str()));
    
  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();

  //this->Image = reader->GetOutput();
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(reader->GetOutput(), this->Image);
  
  Helpers::ITKImagetoVTKImage(this->Image, this->VTKImage);
  
  this->Inpainting.SetImage(this->Image);
    
  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
  
  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);
}


void Form::on_actionOpenMaskInverted_activated()
{
  std::cout << "on_actionOpenMaskInverted_activated()" << std::endl;
  on_actionOpenMask_activated();
  this->MaskImage->Invert();
  this->MaskImage->Cleanup();
  
  this->Inpainting.SetMask(this->MaskImage);
  if(this->DebugImages)
    {
    Helpers::WriteImage<Mask>(this->MaskImage, "Debug/InvertedMask.png");
    }
}

void Form::on_chkImage_clicked()
{
  RefreshSlot();
}

void Form::on_chkMask_clicked()
{
  RefreshSlot();
}

void Form::on_chkPriority_clicked()
{
  RefreshSlot();
}

void Form::on_chkConfidence_clicked()
{
  RefreshSlot();
}

void Form::on_chkBoundary_clicked()
{
  RefreshSlot();
}

void Form::on_chkIsophotes_clicked()
{
  RefreshSlot();
}

void Form::on_chkData_clicked()
{
  RefreshSlot();
}

void Form::on_chkBoundaryNormals_clicked()
{
  RefreshSlot();
}

void Form::on_actionOpenMask_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.bmp)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  typedef itk::ImageFileReader<Mask> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();
  
  if(this->Image->GetLargestPossibleRegion() != reader->GetOutput()->GetLargestPossibleRegion())
    {
    std::cerr << "Image and mask must be the same size!" << std::endl;
    return;
    }

  Helpers::DeepCopy<Mask>(reader->GetOutput(), this->MaskImage);
  
  // For this program, we ALWAYS assume the hole to be filled is white, and the valid/source region is black.
  // This is not simply reversible because of some subtle erosion operations that are performed.
  // For this reason, we provide an "load inverted mask" action in the file menu.
  this->MaskImage->SetValidValue(0);
  this->MaskImage->SetHoleValue(255);
  
  this->MaskImage->Cleanup();

  // This is only set here so we can visualize the mask right away
  this->Inpainting.SetMask(this->MaskImage);
  
  this->statusBar()->showMessage("Opened mask.");
}

void Form::RefreshSlot()
{
  DebugMessage("RefreshSlot()");

  this->ImageSlice->VisibilityOff();
  this->ConfidenceImageSlice->VisibilityOff();
  this->PriorityImageSlice->VisibilityOff();
  this->BoundaryImageSlice->VisibilityOff();
  this->DataImageSlice->VisibilityOff();
  this->BoundaryNormalsActor->VisibilityOff();
  this->IsophoteActor->VisibilityOff();
  this->MaskImageSlice->VisibilityOff();

  if(this->chkImage->isChecked())
    {
    this->ImageSlice->VisibilityOn();
    Helpers::ITKImagetoVTKImage(this->Inpainting.GetResult(), this->VTKImage);
    }
  else if(this->chkMask->isChecked())
    {
    this->MaskImageSlice->VisibilityOn();
    Helpers::ITKScalarImageToScaledVTKImage<Mask>(this->Inpainting.GetMaskImage(), this->VTKMaskImage);
    }
  else if(this->chkConfidence->isChecked())
    {
    this->ConfidenceImageSlice->VisibilityOn();
    Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->Inpainting.GetConfidenceImage(), this->VTKConfidenceImage);
    }
  else if(this->chkPriority->isChecked())
    {
    this->PriorityImageSlice->VisibilityOn();
    Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->Inpainting.GetPriorityImage(), this->VTKPriorityImage);
    }
  else if(this->chkBoundary->isChecked())
    {
    this->BoundaryImageSlice->VisibilityOn();
    Helpers::ITKScalarImageToScaledVTKImage<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), this->VTKBoundaryImage);
    }
  else if(this->chkIsophotes->isChecked())
    {
    if(this->Inpainting.GetIsophoteImage()->GetLargestPossibleRegion().GetSize()[0] != 0)
      {
      // Mask the isophotes image with the current boundary, because we only want to display the isophotes we are interested in.
      FloatVector2ImageType::Pointer normalizedIsophotes = FloatVector2ImageType::New();
      Helpers::DeepCopy<FloatVector2ImageType>(this->Inpainting.GetIsophoteImage(), normalizedIsophotes);
      Helpers::NormalizeVectorImage(normalizedIsophotes);
    
      typedef itk::MaskImageFilter< FloatVector2ImageType, UnsignedCharScalarImageType, FloatVector2ImageType> MaskFilterType;
      typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
      maskFilter->SetInput(normalizedIsophotes);
      maskFilter->SetMaskImage(this->Inpainting.GetBoundaryImage());
      FloatVector2ImageType::PixelType zero;
      zero.Fill(0);
      maskFilter->SetOutsideValue(zero);
      maskFilter->Update();
      
      if(this->DebugImages)
	{
	Helpers::WriteImage<FloatVector2ImageType>(maskFilter->GetOutput(), "Debug/ShowIsophotes.BoundaryIsophotes.mha");
	Helpers::WriteImage<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), "Debug/ShowIsophotes.Boundary.mha");
	}

      Helpers::ConvertNonZeroPixelsToVectors(maskFilter->GetOutput(), this->VTKNonZeroIsophoteVectors);
      
      if(this->DebugImages)
	{
	vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
	writer->SetFileName("Debug/VTKIsophotes.vti");
	writer->SetInputConnection(this->VTKIsophoteImage->GetProducerPort());
	writer->Write();
      
	vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	polyDataWriter->SetFileName("Debug/VTKIsophotes.vtp");
	polyDataWriter->SetInputConnection(this->VTKNonZeroIsophoteVectors->GetProducerPort());
	polyDataWriter->Write();
	}
      }
    this->IsophoteActor->VisibilityOn();
    }
  else if(this->chkData->isChecked())
    {
    this->DataImageSlice->VisibilityOn();
    Helpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->Inpainting.GetDataImage(), this->VTKDataImage);
    }
  else if(this->chkBoundaryNormals->isChecked())
    {
    this->BoundaryNormalsActor->VisibilityOn();
  
    if(this->Inpainting.GetBoundaryNormalsImage()->GetLargestPossibleRegion().GetSize()[0] != 0)
      {
      Helpers::ConvertNonZeroPixelsToVectors(this->Inpainting.GetBoundaryNormalsImage(), this->VTKNonZeroBoundaryNormals);
    
      if(this->DebugImages)
	{
	std::cout << "Writing boundary normals..." << std::endl;
      
	Helpers::WriteImage<FloatVector2ImageType>(this->Inpainting.GetBoundaryNormalsImage(), "Debug/RefreshSlot.BoundaryNormals.mha");
      
	vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
	writer->SetFileName("Debug/RefreshSlot.VTKBoundaryNormals.vti");
	writer->SetInputConnection(this->VTKBoundaryNormalsImage->GetProducerPort());
	writer->Write();
      
	vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	polyDataWriter->SetFileName("Debug/RefreshSlot.VTKBoundaryNormals.vtp");
	polyDataWriter->SetInputConnection(this->VTKNonZeroBoundaryNormals->GetProducerPort());
	polyDataWriter->Write();
	}
      }
    }
    
  Refresh();

  if(this->DebugImages)
  {
    {
    typedef itk::MinimumMaximumImageCalculator <FloatScalarImageType> ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(this->Inpainting.GetPriorityImage());
    imageCalculatorFilter->Compute();
    DebugMessage<float>("Highest priority: ", imageCalculatorFilter->GetMaximum());
    }

    {
    typedef itk::MinimumMaximumImageCalculator <FloatScalarImageType> ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(this->Inpainting.GetConfidenceImage());
    imageCalculatorFilter->Compute();
    DebugMessage<float>("Highest confidence: ", imageCalculatorFilter->GetMaximum());
    }

    {
    typedef itk::MinimumMaximumImageCalculator <FloatScalarImageType> ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(this->Inpainting.GetDataImage());
    imageCalculatorFilter->Compute();
    DebugMessage<float>("Highest data: ", imageCalculatorFilter->GetMaximum());
    }
  }
  
}

void Form::Refresh()
{
  DebugMessage("Refresh()");
  
  this->qvtkWidget->GetRenderWindow()->Render();
  //this->Renderer->Render();
  
}

void Form::on_btnStop_clicked()
{
  this->ComputationThread.StopInpainting();
}

void Form::on_btnReset_clicked()
{
  this->Inpainting.SetImage(this->Image);
  this->Inpainting.SetMask(this->MaskImage);
  RefreshSlot();
}
  

void Form::on_btnInpaint_clicked()
{
  DebugMessage("on_btnInpaint_clicked()");
  
  // Reset some things (this is so that if we want to run another completion it will work normally)

  this->Inpainting.SetPatchRadius(this->txtPatchRadius->text().toUInt());
  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->Inpainting.SetDebugMessages(this->chkDebugMessages->isChecked());
  this->Inpainting.SetImage(this->Image);
  this->Inpainting.SetMask(this->MaskImage);
  
  RefreshSlot();
  
  DebugMessage("Starting ComputationThread...");
  ComputationThread.start();
}

void Form::SetCameraPosition1()
{
  double leftToRight[3] = {-1,0,0};
  double bottomToTop[3] = {0,1,0};
  SetCameraPosition(leftToRight, bottomToTop);
}

void Form::SetCameraPosition2()
{
  double leftToRight[3] = {-1,0,0};
  double bottomToTop[3] = {0,-1,0};

  SetCameraPosition(leftToRight, bottomToTop);
}

void Form::SetCameraPosition(double leftToRight[3], double bottomToTop[3])
{
  this->InteractorStyle->SetImageOrientation(leftToRight, bottomToTop);

  this->Renderer->ResetCamera();
  this->Renderer->ResetCameraClippingRange();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void Form::on_actionFlipImage_activated()
{
  if(this->Flipped)
    {
    SetCameraPosition1();
    }   
  else
    {
    SetCameraPosition2();
    }
  this->Flipped = !this->Flipped;
}


void Form::DebugMessage(const std::string& message)
{
  if(this->DebugMessages)
    {
    std::cout << message << std::endl;
    }
}

void Form::on_btnPrevious_clicked()
{
  if(this->CurrentUsedPatchDisplayed > 0)
    {
    this->CurrentUsedPatchDisplayed--;
    }
  DisplayUsedPatches();
}

void Form::on_btnNext_clicked()
{
  std::cout << "CurrentUsedPatchDisplayed: " << this->CurrentUsedPatchDisplayed << " Inpainting iteration: " <<  static_cast<int>(this->Inpainting.GetIteration()) << std::endl;
  
  if(this->CurrentUsedPatchDisplayed < static_cast<int>(this->Inpainting.GetIteration()) - 1)
    {
    this->CurrentUsedPatchDisplayed++;
    DisplayUsedPatches();
    }

}

void Form::DisplayUsedPatches()
{
  unsigned int patchSize = this->txtPatchRadius->text().toUInt() * 2 + 1;
  
  // Target
  Patch targetPatch;
  bool validTargetPatch = this->Inpainting.GetUsedTargetPatch(this->CurrentUsedPatchDisplayed, targetPatch);
  if(validTargetPatch)
    {
    std::cout << "Displaying used target patch " << this->CurrentUsedPatchDisplayed << " : " << targetPatch.Region << std::endl;
    this->TargetPatch->SetDimensions(patchSize, patchSize, 1);
    Helpers::BlankAndOutlineImage(this->TargetPatch, this->Red);
    this->TargetPatchSlice->SetPosition(targetPatch.Region.GetIndex()[0], targetPatch.Region.GetIndex()[1], 0);
    }
  
  // Source
  Patch sourcePatch;
  bool validSourcePatch = this->Inpainting.GetUsedSourcePatch(this->CurrentUsedPatchDisplayed, sourcePatch);
  if(validSourcePatch)
    std::cout << "Displaying used source patch " << this->CurrentUsedPatchDisplayed << " : " << sourcePatch.Region << std::endl;
    {
    this->SourcePatch->SetDimensions(patchSize, patchSize, 1);
    Helpers::BlankAndOutlineImage(this->SourcePatch, this->Green);
    this->SourcePatchSlice->SetPosition(sourcePatch.Region.GetIndex()[0], sourcePatch.Region.GetIndex()[1], 0);
    }
  
  if(validSourcePatch && validTargetPatch)
    {
    std::stringstream ss;
    ss << this->CurrentUsedPatchDisplayed;
    this->lblCurrentUsedPatches->setText(ss.str().c_str());
    }

  Refresh();
}

void Form::IterationCompleteSlot()
{
  //std::cout << "IterationCompleteSlot()" << std::endl;
  
  this->CurrentUsedPatchDisplayed++;
  if(this->chkDisplayUsedPatches->isChecked())
    {
    DisplayUsedPatches();
    }
  Refresh();
}

void Form::on_chkDisplayUsedPatches_clicked()
{
  if(this->chkDisplayUsedPatches->isChecked())
    {
    this->SourcePatchSlice->VisibilityOn();
    this->TargetPatchSlice->VisibilityOn();
    }
  else
    {
    this->SourcePatchSlice->VisibilityOff();
    this->TargetPatchSlice->VisibilityOff();
    }
}
