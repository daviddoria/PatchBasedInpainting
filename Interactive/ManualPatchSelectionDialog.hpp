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

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/ITKVTKHelpers.h"
#include "Helpers/VTKHelpers.h"
#include "Interactive/HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"
#include "ImageProcessing/Mask.h"

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
  zeroPixel.Fill(0);
  mask->ApplyToImage(tempImage.GetPointer(), zeroPixel);
  ITKVTKHelpers::ITKVectorImageToVTKImageFromDimension(tempImage, this->ImageLayer.ImageData);

  SetupScenes();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();

  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(ImageLayer.ImageSlice);

  // Per the comment in InteractorStyleImageWithDrag, the next 3 lines must be in this order
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  this->PatchSelector = new MovablePatch(this->TargetRegion.GetSize()[0]/2, this->InteractorStyle, this->gfxSource);

  // slot_UpdateImage();

  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
  // this->Camera = new ImageCamera(this->Renderer);

  connect(this->PatchSelector, SIGNAL(signal_PatchMoved()), this, SLOT(slot_PatchMoved()));
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::slot_PatchMoved()
{
  slot_UpdateSource(PatchSelector->GetRegion(), TargetRegion);
  slot_UpdateResult(PatchSelector->GetRegion(), TargetRegion);

  // This will refresh the scene so that the old patch positions are erased
  //this->InteractorStyle->GetCurrentRenderer()->GetRenderWindow()->Render(); // (this doesn't work...)
  this->qvtkWidget->GetRenderWindow()->Render();
}

// template <typename TImage>
// void ManualPatchSelectionDialog<TImage>::slot_UpdateImage()
// {
//   std::cout << "Update image." << std::endl;
//   //ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image, this->ImageLayer.ImageData);
//   unsigned char green[3] = {0, 255, 0};
//   ITKVTKHelpers::ITKImageToVTKImageMasked(this->Image, this->MaskImage,
//                                           this->ImageLayer.ImageData, green);
//   int dims[3];
//   this->ImageLayer.ImageData->GetDimensions(dims);
//   if(dims[0] != ImageDimension[0] || dims[1] != ImageDimension[1] || dims[2] != ImageDimension[2])
//     {
//     this->Renderer->ResetCamera();
//     ImageDimension[0] = dims[0];
//     ImageDimension[1] = dims[1];
//     ImageDimension[2] = dims[2];
//     }
// 
//   this->qvtkWidget->GetRenderWindow()->Render();
// }

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::SetupScenes()
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
  
  if(MaskImage->CountHolePixels(sourceRegion) > 0)
  {
    std::cerr << "The source patch must not have any hole pixels!" << std::endl;
    btnAccept->setVisible(false);
  }
  else
  {
    btnAccept->setVisible(true);
  }

  typename TImage::Pointer tempImage = TImage::New();
  ITKHelpers::ConvertTo3Channel(this->Image, tempImage.GetPointer());
  
  QImage maskedSourceImage = HelpersQt::GetQImageMasked(tempImage.GetPointer(), sourceRegion, MaskImage, sourceRegion);
  QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(maskedSourceImage));
  gfxSource->fitInView(item);

  // Refresh the image
  //ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image, this->ImageLayer.ImageData);
  unsigned char green[3] = {0, 255, 0};

  ITKVTKHelpers::ITKImageToVTKImageMasked(tempImage, this->MaskImage,
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
  
  QImage maskedTargetImage = HelpersQt::GetQImageMasked(tempImage.GetPointer(), MaskImage, region);
  QGraphicsPixmapItem* maskedItem = this->TargetPatchScene->addPixmap(QPixmap::fromImage(maskedTargetImage));
  gfxTarget->fitInView(maskedItem);
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
      } // end iterator loop

    qimage = HelpersQt::GetQImageColor(resultPatch.GetPointer(), resultPatch->GetLargestPossibleRegion());
  } // end else

  this->ResultPatchScene->clear();
  QGraphicsPixmapItem* item = this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));
  gfxResult->fitInView(item);

}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::on_btnAccept_clicked()
{
  itk::Index<2> patchCenter = ITKHelpers::GetRegionCenter(PatchSelector->GetRegion());
  SelectedNode = Helpers::ConvertFrom<Node, itk::Index<2> >(patchCenter);
  accept();
}

template <typename TImage>
void ManualPatchSelectionDialog<TImage>::showEvent(QShowEvent* event)
{
  slot_UpdateTarget(TargetRegion);
}

template <typename TImage>
Node ManualPatchSelectionDialog<TImage>::GetSelectedNode()
{
  return SelectedNode;
}

#endif
