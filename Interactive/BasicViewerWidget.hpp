/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef BasicViewerWidget_HPP
#define BasicViewerWidget_HPP

#include "BasicViewerWidget.h" // Appease syntax parser

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QGraphicsPixmapItem>
#include <QFileDialog>

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

// Submodules
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>
#include <VTKHelpers/VTKHelpers.h>
#include <QtHelpers/QtHelpers.h>
#include <ITKQtHelpers/ITKQtHelpers.h>
#include <Mask/Mask.h>
#include <Mask/MaskOperations.h>

// Custom
#include "InteractorStyleImageWithDrag.h"

template <typename TImage>
//BasicViewerWidget<TImage>::BasicViewerWidget(TImage* const image, Mask* const mask) :
BasicViewerWidget<TImage>::BasicViewerWidget(typename TImage::Pointer image, Mask::Pointer mask) :
  Image(image), MaskImage(mask)
{
  qRegisterMetaType<itk::ImageRegion<2> >("itkImageRegion");

  this->setupUi(this);

  int dims[3];
  this->ImageLayer.ImageData->GetDimensions(dims);

  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::DeepCopy(image.GetPointer(), tempImage.GetPointer());

  typename TImage::PixelType zeroPixel(tempImage->GetNumberOfComponentsPerPixel());
  zeroPixel = itk::NumericTraits<typename TImage::PixelType>::ZeroValue(zeroPixel);
  mask->ApplyToImage(tempImage.GetPointer(), zeroPixel);
  ITKVTKHelpers::ITKVectorImageToVTKImageFromDimension(tempImage.GetPointer(), this->ImageLayer.ImageData);

//   if(chkScaleImage->isChecked())
//   {
//     VTKHelpers::ScaleImage(this->ImageLayer.ImageData);
//   }

  this->ImageDimension[0] = dims[0];
  this->ImageDimension[1] = dims[1];
  this->ImageDimension[2] = dims[2];

  SetupScenes();

//  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();
  this->InteractorStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(ImageLayer.ImageSlice);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
//  this->InteractorStyle->Init();

//  this->Camera = new ImageCamera(this->Renderer);
  this->ItkVtkCamera = new ITKVTKCamera(this->InteractorStyle, this->Renderer,
                                        this->qvtkWidget->GetRenderWindow());
  this->ItkVtkCamera->SetCameraPositionPNG();
}

template <typename TImage>
void BasicViewerWidget<TImage>::SetupScenes()
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
}

template <typename TImage>
void BasicViewerWidget<TImage>::slot_UpdateImage()
{
//  std::cout << "BasicViewerWidget::slot_UpdateImage()" << std::endl;

  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::DeepCopy(this->Image.GetPointer(), tempImage.GetPointer());

  typename TImage::PixelType zeroPixel(tempImage->GetNumberOfComponentsPerPixel());
  zeroPixel = itk::NumericTraits<typename TImage::PixelType>::ZeroValue(zeroPixel);

  this->MaskImage->ApplyToImage(tempImage.GetPointer(), zeroPixel);
  ITKVTKHelpers::ITKVectorImageToVTKImageFromDimension(tempImage.GetPointer(), this->ImageLayer.ImageData);

//   if(chkScaleImage->isChecked())
//   {
//     std::cout << "scaling..." << std::endl;
//     VTKHelpers::ScaleImage(this->ImageLayer.ImageData);
//   }
  
  int dims[3];
  this->ImageLayer.ImageData->GetDimensions(dims);
  if(dims[0] != ImageDimension[0] || dims[1] != ImageDimension[1] || dims[2] != ImageDimension[2])
  {
    this->Renderer->ResetCamera();
    ImageDimension[0] = dims[0];
    ImageDimension[1] = dims[1];
    ImageDimension[2] = dims[2];
  }

  this->qvtkWidget->GetRenderWindow()->Render();
}

template <typename TImage>
void BasicViewerWidget<TImage>::slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion,
                                                  const itk::ImageRegion<2>& targetRegion)
{
  // This function needs the targetRegion because this is the region of the Mask that is used to mask the source patch.
//  std::cout << "BasicViewerWidget::slot_UpdateSource " << sourceRegion << std::endl;

  slot_UpdateSource(sourceRegion);
}

template <typename TImage>
void BasicViewerWidget<TImage>::slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion)
{
//  std::cout << "BasicViewerWidget::slot_UpdateSource " << sourceRegion << std::endl;

  if(!this->SourceHighlighter)
  {
    this->SourceHighlighter = new PatchHighlighter(sourceRegion.GetSize()[0]/2, this->Renderer, Qt::green);
  }
  this->SourceHighlighter->SetRegion(sourceRegion);

  QImage sourceImage = ITKQtHelpers::GetQImageColor(this->Image.GetPointer(), sourceRegion);
  QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));
  gfxSource->fitInView(item);

  // Make the renderer show the Highlighter in the new position
  this->qvtkWidget->GetRenderWindow()->Render();
}

template <typename TImage>
void BasicViewerWidget<TImage>::slot_UpdateTarget(const itk::ImageRegion<2>& targetRegion)
{
//  std::cout << "BasicViewerWidget:slot_UpdateTarget " << targetRegion << std::endl;

  if(!this->TargetHighlighter)
  {
    this->TargetHighlighter = new PatchHighlighter(targetRegion.GetSize()[0]/2, this->Renderer, Qt::red);
  }

  this->TargetHighlighter->SetRegion(targetRegion);

  // Target patch
  QImage targetImage = ITKQtHelpers::GetQImageColor(this->Image.GetPointer(), targetRegion);
  QGraphicsPixmapItem* item = this->TargetPatchScene->addPixmap(QPixmap::fromImage(targetImage));
  gfxTarget->fitInView(item);

  // Make the renderer show the Highlighter in the new position
  this->qvtkWidget->GetRenderWindow()->Render();
}

template <typename TImage>
template <typename TVisitor>
void BasicViewerWidget<TImage>::ConnectVisitor(TVisitor* visitor)
{
  // These are all BlockingQueuedConnection because the 'visitor' is not in the GUI thread.

  QObject::connect(visitor, SIGNAL(signal_RefreshImage()),
                   this, SLOT(slot_UpdateImage()),
                   Qt::BlockingQueuedConnection);

  QObject::connect(visitor, SIGNAL(signal_RefreshSource(const itk::ImageRegion<2>&,
                                                        const itk::ImageRegion<2>&)),
                   this, SLOT(slot_UpdateSource(const itk::ImageRegion<2>&,
                                                const itk::ImageRegion<2>&)),
                   Qt::BlockingQueuedConnection);

  QObject::connect(visitor, SIGNAL(signal_RefreshTarget(const itk::ImageRegion<2>&)),
                   this, SLOT(slot_UpdateTarget(const itk::ImageRegion<2>&)),
                   Qt::BlockingQueuedConnection);

}

template <typename TImage>
template <typename TPatchesWidget>
void BasicViewerWidget<TImage>::ConnectWidget(TPatchesWidget* widget)
{
  // This is a direct connection because the 'widget' must already be in the GUI thread.
  QObject::connect(widget, SIGNAL(signal_SelectedRegion(const itk::ImageRegion<2>&)),
                   this, SLOT(slot_UpdateSource(const itk::ImageRegion<2>&)),
                   Qt::DirectConnection);
}

template <typename TImage>
void BasicViewerWidget<TImage>::closeEvent(QCloseEvent*)
{
  std::cout << "Quitting..." << std::endl;
  QApplication::quit();
}

template <typename TImage>
void BasicViewerWidget<TImage>::on_actionSave_triggered()
{
  QString fileName = QFileDialog::getSaveFileName(this,
     "Save Image", ".", "Image Files (*.png)");

//  ITKHelpers::WriteImage(this->Image.GetPointer(), fileName.toStdString());
  ITKHelpers::WriteRGBImage(this->Image.GetPointer(), fileName.toStdString());
  std::cout << "Wrote image " << fileName.toStdString() << std::endl;
}

template <typename TImage>
void BasicViewerWidget<TImage>::on_actionQuit_triggered()
{
  std::cout << "Quit." << std::endl;
}

#endif
