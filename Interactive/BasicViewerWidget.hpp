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
#include "Helpers/OutputHelpers.h"
#include "Helpers/ITKVTKHelpers.h"
#include "Interactive/HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"
#include "ImageProcessing/Mask.h"

template <typename TImage>
BasicViewerWidget<TImage>::BasicViewerWidget(TImage* const image, Mask* const mask) : Image(image), MaskImage(mask)
{
  qRegisterMetaType<itk::ImageRegion<2> >("itkImageRegion");

  this->setupUi(this);

  this->ImageDimension[0] = 0;
  this->ImageDimension[1] = 0;
  this->ImageDimension[2] = 0;

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
  ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image, this->ImageLayer.ImageData);

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
  std::cout << "Update source." << std::endl;

  QImage sourceImage = HelpersQt::GetQImageColor(Image, sourceRegion);
  QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));
  gfxSource->fitInView(item);

  QImage maskedSourceImage = HelpersQt::GetQImageMasked(Image, sourceRegion, MaskImage, targetRegion);
  QGraphicsPixmapItem* maskedItem = this->MaskedSourcePatchScene->addPixmap(QPixmap::fromImage(maskedSourceImage));
  gfxMaskedSource->fitInView(maskedItem);
}

template <typename TImage>
void BasicViewerWidget<TImage>::slot_UpdateTarget(const itk::ImageRegion<2>& region)
{
  std::cout << "Update target." << std::endl;

  // Target patch
  QImage targetImage = HelpersQt::GetQImageColor(Image, region);
  QGraphicsPixmapItem* item = this->TargetPatchScene->addPixmap(QPixmap::fromImage(targetImage));
  gfxTarget->fitInView(item);

  // Masked target patch
  QImage maskedTargetImage = HelpersQt::GetQImageMasked(Image, MaskImage, region);
  QGraphicsPixmapItem* maskedItem = this->MaskedTargetPatchScene->addPixmap(QPixmap::fromImage(maskedTargetImage));
  gfxMaskedTarget->fitInView(maskedItem);
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
    FloatVectorImageType::PixelType pixel;

    if(MaskImage->IsHole(maskIterator.GetIndex()))
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

  qimage = HelpersQt::GetQImageColor(resultPatch.GetPointer(), resultPatch->GetLargestPossibleRegion());

  this->ResultPatchScene->clear();
  QGraphicsPixmapItem* item = this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));
  gfxResult->fitInView(item);

}

#endif
