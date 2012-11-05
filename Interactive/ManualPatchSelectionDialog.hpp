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

#ifndef ManualPatchSelectionDialog_HPP
#define ManualPatchSelectionDialog_HPP

#include "ManualPatchSelectionDialog.h" // Appease syntax parser

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QGraphicsPixmapItem>

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
#include <QVTKWidget.h>

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
ManualPatchSelectionDialog<TImage>::ManualPatchSelectionDialog(TImage* const image, Mask* const mask,
                                                               const itk::ImageRegion<2>& targetRegion)
: Image(image), MaskImage(mask), TargetRegion(targetRegion)
{
  // Allow the type itkImageRegion to be used in signals/slots.
  qRegisterMetaType<itk::ImageRegion<2> >("itkImageRegion");

  this->setupUi(this);

  this->ImageLayer.ImageSlice->SetDragable(false);
  this->ImageLayer.ImageSlice->SetPickable(false);

  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::DeepCopy(image, tempImage.GetPointer());
  typename TImage::PixelType zeroPixel(tempImage->GetNumberOfComponentsPerPixel());

  zeroPixel = itk::NumericTraits<typename TImage::PixelType>::ZeroValue(zeroPixel);
  mask->ApplyToImage(tempImage.GetPointer(), zeroPixel);
  ITKVTKHelpers::ITKVectorImageToVTKImageFromDimension(tempImage.GetPointer(),
                                                       this->ImageLayer.ImageData);

  SetupScenes();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();

  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);

  // Per the comment in InteractorStyleImageWithDrag, the next 3 lines must be in this order
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  this->PatchSelector = new MovablePatch(this->TargetRegion.GetSize()[0]/2,
                                         this->InteractorStyle, this->gfxSource);

  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
  // this->Camera = new ImageCamera(this->Renderer);

  // Setup the viewing orientation
  this->ItkVtkCamera = new ITKVTKCamera(this->InteractorStyle->GetImageStyle(), this->Renderer,
                                        this->qvtkWidget->GetRenderWindow());
  this->ItkVtkCamera->SetCameraPositionPNG();

  connect(this->PatchSelector, SIGNAL(signal_PatchMoved()), this, SLOT(slot_PatchMoved()));
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::slot_PatchMoved()
{
  slot_UpdateSource(this->PatchSelector->GetRegion(), this->TargetRegion);
  slot_UpdateResult(this->PatchSelector->GetRegion(), this->TargetRegion);

  // This will refresh the scene so that the old patch positions are erased
  //this->InteractorStyle->GetCurrentRenderer()->GetRenderWindow()->Render(); // (this doesn't work...)
  this->qvtkWidget->GetRenderWindow()->Render();
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::SetupScenes()
{
  // Set the color of the background of the target/source/result scenes
  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(this->SceneBackground);

  // Setup the target scene
  this->TargetPatchScene = new QGraphicsScene();
  this->TargetPatchScene->setBackgroundBrush(brush);
  this->gfxTarget->setScene(TargetPatchScene);

  // Setup the source scene
  this->SourcePatchScene = new QGraphicsScene();
  this->SourcePatchScene->setBackgroundBrush(brush);
  this->gfxSource->setScene(SourcePatchScene);

  // Setup the result scene
  this->ResultPatchScene = new QGraphicsScene();
  this->ResultPatchScene->setBackgroundBrush(brush);
  this->gfxResult->setScene(ResultPatchScene);
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion,
                                                           const itk::ImageRegion<2>& targetRegion)
{
  // This function needs the targetRegion because this is the region of the Mask that is used to mask the source patch.
  // std::cout << "Update source." << std::endl;

  if(!this->Image->GetLargestPossibleRegion().IsInside(sourceRegion))
  {
    std::cerr << "Source region is outside the image!" << std::endl;
    return;
  }
  
  if(this->MaskImage->CountHolePixels(sourceRegion) > 0)
  {
    std::cerr << "The source patch must not have any hole pixels!" << std::endl;
    this->btnAccept->setVisible(false);
  }
  else
  {
    this->btnAccept->setVisible(true);
  }

  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::ConvertTo3Channel(this->Image, tempImage.GetPointer());

  typename TImage::PixelType zeroPixel(3);
  zeroPixel = itk::NumericTraits<typename TImage::PixelType>::ZeroValue(zeroPixel);

  this->MaskImage->ApplyToImage(tempImage.GetPointer(), zeroPixel);
  QImage maskedSourceImage = ITKQtHelpers::GetQImageColor(tempImage.GetPointer(), sourceRegion);
  QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(maskedSourceImage));
  this->gfxSource->fitInView(item);

  // Refresh the image
  //ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image, this->ImageLayer.ImageData);
  unsigned char green[3] = {0, 255, 0};

  MaskOperations::ITKImageToVTKImageMasked(tempImage.GetPointer(), this->MaskImage,
                                          this->ImageLayer.ImageData, green);

  // Outline the source patch
  unsigned char blue[3] = {0, 0, 255};
  ITKVTKHelpers::OutlineRegion(this->ImageLayer.ImageData, sourceRegion, blue);

  this->qvtkWidget->GetRenderWindow()->Render();
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::slot_UpdateTarget(const itk::ImageRegion<2>& region)
{
  // std::cout << "Update target." << std::endl;

  unsigned char red[3] = {255, 0, 0};
  ITKVTKHelpers::OutlineRegion(this->ImageLayer.ImageData, region, red);

  this->qvtkWidget->GetRenderWindow()->Render();

  // Masked target patch
  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::ConvertTo3Channel(this->Image, tempImage.GetPointer());
  typename TImage::PixelType zeroPixel(3);
  zeroPixel = itk::NumericTraits<typename TImage::PixelType>::ZeroValue(zeroPixel);

  this->MaskImage->ApplyToImage(tempImage.GetPointer(), zeroPixel);
  QImage maskedTargetImage = ITKQtHelpers::GetQImageColor(tempImage.GetPointer(), region);
  QGraphicsPixmapItem* maskedItem = this->TargetPatchScene->addPixmap(QPixmap::fromImage(maskedTargetImage));
  this->gfxTarget->fitInView(maskedItem);
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion,
                                                           const itk::ImageRegion<2>& targetRegion)
{
  assert(sourceRegion.GetSize() == targetRegion.GetSize());

  if(!this->Image->GetLargestPossibleRegion().IsInside(sourceRegion))
  {
    std::cerr << "Source region is outside the image!" << std::endl;
    return;
  }
  
  QImage qimage(sourceRegion.GetSize()[0], sourceRegion.GetSize()[1], QImage::Format_RGB888);

  if(MaskImage->CountHolePixels(sourceRegion) > 0)
  {
    //std::cerr << "The source patch must not have any hole pixels!" << std::endl;
    //btnAccept->setVisible(false);
    qimage.fill(Qt::green);
  }
  else
  {
    typename TImage::Pointer tempImage = TImage::New();
    ITKHelpers::ConvertTo3Channel(this->Image, tempImage.GetPointer());

    // If the original image was not 3 channels, it was not RGB and we probably want to scale it
    if(this->Image->GetNumberOfComponentsPerPixel() != 3)
    {
      ITKHelpers::ScaleAllChannelsTo255(tempImage.GetPointer());
    }

    itk::ImageRegionIterator<TImage> sourceIterator(tempImage, sourceRegion);
    itk::ImageRegionIterator<TImage> targetIterator(tempImage, targetRegion);
    itk::ImageRegionIterator<Mask> maskIterator(MaskImage, targetRegion);

    typename TImage::Pointer resultPatch = TImage::New();
    resultPatch->SetNumberOfComponentsPerPixel(Image->GetNumberOfComponentsPerPixel());
    itk::ImageRegion<2> resultPatchRegion;
    resultPatchRegion.SetSize(sourceRegion.GetSize());
    resultPatch->SetRegions(resultPatchRegion);
    resultPatch->Allocate();

    while(!maskIterator.IsAtEnd())
    {
      typename TImage::PixelType pixel;

      if(this->MaskImage->IsHole(maskIterator.GetIndex()))
      {
        pixel = sourceIterator.Get();
      }
      else
      {
        pixel = targetIterator.Get();
      }

      itk::Offset<2> offset = sourceIterator.GetIndex() - sourceRegion.GetIndex();
      itk::Index<2> offsetIndex = {{offset[0], offset[1]}};
      resultPatch->SetPixel(offsetIndex, pixel);

      ++sourceIterator;
      ++targetIterator;
      ++maskIterator;
    } // end iterator loop

    qimage = ITKQtHelpers::GetQImageColor(resultPatch.GetPointer(), resultPatch->GetLargestPossibleRegion());
  } // end else

  this->ResultPatchScene->clear();
  QGraphicsPixmapItem* item = this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));
  this->gfxResult->fitInView(item);
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::on_btnAccept_clicked()
{
  // Store the result so it can be accessed from the caller who opened the dialog
  itk::Index<2> patchCenter = ITKHelpers::GetRegionCenter(this->PatchSelector->GetRegion());
  this->SelectedNode = Helpers::ConvertFrom<Node, itk::Index<2> >(patchCenter);

  // Return from the dialog
  accept();
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::showEvent(QShowEvent* event)
{
  slot_UpdateTarget(this->TargetRegion);
}

template <typename TImage>
Node ManualPatchSelectionDialog<TImage>::GetSelectedNode()
{
  return this->SelectedNode;
}

#endif
