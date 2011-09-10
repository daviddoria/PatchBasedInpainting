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

#include "ui_Form.h"
#include "Form.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QFileDialog>
#include <QIcon>
#include <QTextEdit>

// VTK
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
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
#include <vtkVertexGlyphFilter.h>
#include <vtkXMLPolyDataReader.h>

// Custom
#include "EventThrower.h"
#include "Helpers.h"
#include "Types.h"

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

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");
  QIcon saveIcon = QIcon::fromTheme("document-save");
  
  // Setup toolbar
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  actionOpenMask->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMask);

  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);

  this->Flipped = false;
  
  this->InteractorStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
  
  // Initialize and link the image display objects
  this->VTKImage = vtkSmartPointer<vtkImageData>::New();
  this->ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  
  this->ImageSliceMapper->SetInputConnection(this->VTKImage->GetProducerPort());
  this->ImageSlice->SetMapper(this->ImageSliceMapper);
  
  
  // Initialize and link the priority image display objects
  this->VTKPriorityImage = vtkSmartPointer<vtkImageData>::New();
  this->PriorityImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->PriorityImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  
  this->PriorityImageSliceMapper->SetInputConnection(this->VTKPriorityImage->GetProducerPort());
  this->PriorityImageSlice->SetMapper(this->PriorityImageSliceMapper);
  
  
  // Initialize and link the confidence image display objects
  this->VTKConfidenceImage = vtkSmartPointer<vtkImageData>::New();
  this->ConfidenceImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ConfidenceImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  
  this->ConfidenceImageSliceMapper->SetInputConnection(this->VTKConfidenceImage->GetProducerPort());
  this->ConfidenceImageSlice->SetMapper(this->ConfidenceImageSliceMapper);
  
  // Initialize and link the boundary image display objects
  this->VTKBoundaryImage = vtkSmartPointer<vtkImageData>::New();
  this->BoundaryImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->BoundaryImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  
  this->BoundaryImageSliceMapper->SetInputConnection(this->VTKBoundaryImage->GetProducerPort());
  this->BoundaryImageSlice->SetMapper(this->BoundaryImageSliceMapper);
  
  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);
  
  this->Renderer->AddViewProp(this->ImageSlice);
  this->Renderer->AddViewProp(this->ConfidenceImageSlice);
  this->Renderer->AddViewProp(this->BoundaryImageSlice);
  this->Renderer->AddViewProp(this->PriorityImageSlice);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  
  //this->Image = NULL;
  //this->Mask = NULL;
  
  this->Image = FloatVectorImageType::New();
  this->Mask = UnsignedCharScalarImageType::New();
  
  connect(&ComputationThread, SIGNAL(StartProgressSignal()), this, SLOT(StartProgressSlot()), Qt::QueuedConnection);
  connect(&ComputationThread, SIGNAL(StopProgressSignal()), this, SLOT(StopProgressSlot()), Qt::QueuedConnection);

  // Set the progress bar to marquee mode
  this->progressBar->setMinimum(0);
  this->progressBar->setMaximum(0);
  this->progressBar->hide();
  
  ComputationThread.SetObject(&(this->Inpainting));
  
  connect(&Inpainting, SIGNAL(RefreshSignal()), this, SLOT(RefreshSlot()), Qt::QueuedConnection);
};


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
  std::cout << "Form::StartProgressSlot()" << std::endl;
  // Connected to the StartProgressSignal of the ProgressThread member
  this->progressBar->show();
}

void Form::StopProgressSlot()
{
  std::cout << "Form::StopProgressSlot()" << std::endl;
  // When the ProgressThread emits the StopProgressSignal, we need to display the result of the segmentation

  this->progressBar->hide();
}

void Form::on_actionOpenImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha)");

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
  
  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
  
  this->statusBar()->showMessage("Opened image.");
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

  typedef itk::ImageFileReader<UnsignedCharScalarImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();

  //this->Mask = reader->GetOutput();
  Helpers::DeepCopy<UnsignedCharScalarImageType>(reader->GetOutput(), this->Mask);
  std::cout << "Mask in Form is size: " << this->Mask->GetLargestPossibleRegion() << std::endl;
  
  /*
  Helpers::ITKImagetoVTKImage(this->Image, this->VTKImage);
  
  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
  */
  
  this->statusBar()->showMessage("Opened mask.");
}

void Form::RefreshSlot()
{
  std::cout << "RefreshSlot()" << std::endl;

  this->ImageSlice->VisibilityOff();
  this->ConfidenceImageSlice->VisibilityOff();
  this->PriorityImageSlice->VisibilityOff();
  this->BoundaryImageSlice->VisibilityOff();

  if(this->chkImage->isChecked())
    {
    //std::cout << "chkImage" << std::endl;
    this->ImageSlice->VisibilityOn();
    Helpers::ITKImagetoVTKImage(this->Inpainting.GetResult(), this->VTKImage);
    //this->Renderer->AddViewProp(this->ImageSlice);
    }
  else if(this->chkConfidence->isChecked())
    {
    //std::cout << "chkConfidence" << std::endl;
    this->ConfidenceImageSlice->VisibilityOn();
    Helpers::ITKScalarImagetoVTKImage<FloatScalarImageType>(this->Inpainting.GetConfidenceImage(), this->VTKConfidenceImage);
    //this->Renderer->AddViewProp(this->ConfidenceImageSlice);
    }
  else if(this->chkPriority->isChecked())
    {
    this->PriorityImageSlice->VisibilityOn();
    Helpers::ITKScalarImagetoVTKImage<FloatScalarImageType>(this->Inpainting.GetPriorityImage(), this->VTKPriorityImage);
    //this->Renderer->AddViewProp(this->PriorityImageSlice);
    }
  else if(this->chkBoundary->isChecked())
    {
    this->BoundaryImageSlice->VisibilityOn();
    Helpers::ITKScalarImagetoVTKImage<UnsignedCharScalarImageType>(this->Inpainting.GetBoundaryImage(), this->VTKBoundaryImage);
    //this->Renderer->AddViewProp(this->BoundaryImageSlice);
    }

  Refresh();
}

void Form::Refresh()
{
  std::cout << "Refresh()" << std::endl;
  
  this->qvtkWidget->GetRenderWindow()->Render();
  //this->Renderer->Render();
  
}

void Form::on_btnInpaint_clicked()
{
  std::cout << "on_btnInpaint_clicked()" << std::endl;
  //this->Inpainting.Thrower->AddObserver(EventThrower::RefreshEvent, this, &Form::Refresh);
  
  this->Inpainting.SetPatchRadius(this->txtPatchSize->text().toUInt()/2);
  
  this->Inpainting.SetDebug(true);
  this->Inpainting.SetImage(this->Image);
  this->Inpainting.SetMask(this->Mask);
  //this->Inpainting.Inpaint();
  std::cout << "starting ComputationThread..." << std::endl;
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
