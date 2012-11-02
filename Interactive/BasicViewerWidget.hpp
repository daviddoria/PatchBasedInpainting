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

// Custom
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKHelpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "VTKHelpers/VTKHelpers.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"
#include "InteractorStyleImageWithDrag.h"
#include "Mask/Mask.h"
#include "Mask/MaskOperations.h"

template <typename TImage>
BasicViewerWidget<TImage>::BasicViewerWidget(TImage* const image, Mask* const mask) : Image(image), MaskImage(mask), SourceHighlighter(NULL), TargetHighlighter(NULL)
{
  qRegisterMetaType<itk::ImageRegion<2> >("itkImageRegion");

  this->setupUi(this);

  int dims[3];
  this->ImageLayer.ImageData->GetDimensions(dims);

  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::DeepCopy(image, tempImage.GetPointer());

  typename TImage::PixelType zeroPixel(tempImage->GetNumberOfComponentsPerPixel());
  zeroPixel.Fill(0);
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

  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(ImageLayer.ImageSlice);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  this->Camera = new ImageCamera(this->Renderer);
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

  this->ResultPatchScene = new QGraphicsScene();
  this->ResultPatchScene->setBackgroundBrush(brush);
  this->gfxResult->setScene(ResultPatchScene);

  this->MaskedSourcePatchScene = new QGraphicsScene();
  this->MaskedSourcePatchScene->setBackgroundBrush(brush);
  this->gfxMaskedSource->setScene(MaskedSourcePatchScene);

  this->MaskedTargetPatchScene = new QGraphicsScene();
  this->MaskedTargetPatchScene->setBackgroundBrush(brush);
  this->gfxMaskedTarget->setScene(MaskedTargetPatchScene);
}

template <typename TImage>
void BasicViewerWidget<TImage>::slot_UpdateImage()
{
  std::cout << "Update image." << std::endl;

  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::DeepCopy(this->Image, tempImage.GetPointer());

  typename TImage::PixelType zeroPixel(tempImage->GetNumberOfComponentsPerPixel());
  zeroPixel.Fill(0);
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
void BasicViewerWidget<TImage>::slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
{
  // This function needs the targetRegion because this is the region of the Mask that is used to mask the source patch.
  std::cout << "Update source " << sourceRegion << std::endl;

  if(!SourceHighlighter)
    {
    SourceHighlighter = new PatchHighlighter(sourceRegion.GetSize()[0]/2, this->Renderer, Qt::green);
    }
  SourceHighlighter->SetRegion(sourceRegion);
  
  QImage sourceImage = ITKQtHelpers::GetQImageColor(Image, sourceRegion);
  QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));
  gfxSource->fitInView(item);

  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::ConvertTo3Channel(this->Image, tempImage.GetPointer());
  typename TImage::PixelType zeroPixel(3);
  zeroPixel.Fill(0);
  this->MaskImage->ApplyRegionToImageRegion(sourceRegion, tempImage.GetPointer(), targetRegion, zeroPixel);
  
  QImage maskedSourceImage = ITKQtHelpers::GetQImageColor(tempImage.GetPointer(), sourceRegion);
  QGraphicsPixmapItem* maskedItem = this->MaskedSourcePatchScene->addPixmap(QPixmap::fromImage(maskedSourceImage));
  gfxMaskedSource->fitInView(maskedItem);

  //unsigned char blue[3] = {0, 0, 255};
  //ITKVTKHelpers::OutlineRegion(this->ImageLayer.ImageData, sourceRegion, blue);
  //ITKVTKHelpers::BlankAndOutlineRegion(this->ImageLayer.ImageData, sourceRegion, blue);

  this->qvtkWidget->GetRenderWindow()->Render();
}

template <typename TImage>
void BasicViewerWidget<TImage>::slot_UpdateTarget(const itk::ImageRegion<2>& targetRegion)
{
  std::cout << "Update target " << targetRegion << std::endl;

  if(!TargetHighlighter)
    {
    TargetHighlighter = new PatchHighlighter(targetRegion.GetSize()[0]/2, this->Renderer, Qt::red);
    }

  TargetHighlighter->SetRegion(targetRegion);
  
  //unsigned char red[3] = {255, 0, 0};
  //ITKVTKHelpers::OutlineRegion(this->ImageLayer.ImageData, targetRegion, red);

  // Target patch
  QImage targetImage = ITKQtHelpers::GetQImageColor(Image, targetRegion);
  QGraphicsPixmapItem* item = this->TargetPatchScene->addPixmap(QPixmap::fromImage(targetImage));
  gfxTarget->fitInView(item);

  // Masked target patch
  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::ConvertTo3Channel(this->Image, tempImage.GetPointer());
  typename TImage::PixelType zeroPixel(3);
  zeroPixel.Fill(0);
  this->MaskImage->ApplyToImage(tempImage.GetPointer(), zeroPixel);
  QImage maskedTargetImage = ITKQtHelpers::GetQImageColor(tempImage.GetPointer(), targetRegion);

  QGraphicsPixmapItem* maskedItem = this->MaskedTargetPatchScene->addPixmap(QPixmap::fromImage(maskedTargetImage));
  gfxMaskedTarget->fitInView(maskedItem);

  this->qvtkWidget->GetRenderWindow()->Render();
}

template <typename TImage>
void BasicViewerWidget<TImage>::slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
{
  assert(sourceRegion.GetSize() == targetRegion.GetSize());

  QImage qimage(sourceRegion.GetSize()[0], sourceRegion.GetSize()[1], QImage::Format_RGB888);

  itk::ImageRegionIterator<TImage> sourceIterator(Image, sourceRegion);
  itk::ImageRegionIterator<TImage> targetIterator(Image, targetRegion);
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
    itk::Index<2> offsetIndex;
    offsetIndex[0] = offset[0];
    offsetIndex[1] = offset[1];
    resultPatch->SetPixel(offsetIndex, pixel);

    ++sourceIterator;
    ++targetIterator;
    ++maskIterator;
    }

  qimage = ITKQtHelpers::GetQImageColor(resultPatch.GetPointer(), resultPatch->GetLargestPossibleRegion());

  this->ResultPatchScene->clear();
  QGraphicsPixmapItem* item = this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));
  gfxResult->fitInView(item);

}

#endif
