#ifndef TopPatchesWidget_HPP
#define TopPatchesWidget_HPP


#include "TopPatchesWidget.h" // Appease syntax parser

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QGraphicsPixmapItem>

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/ITKVTKHelpers.h"
#include "Interactive/HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"
#include "ImageProcessing/Mask.h"

template <typename TImage>
TopPatchesWidget<TImage>::TopPatchesWidget(TImage* const image) : Image(image)
{
  qRegisterMetaType<itk::ImageRegion<2> >("itkImageRegion");

  this->setupUi(this);

  SetupScenes();

  PatchesModel = new ListModelPatches<TImage>(image);
  this->listView->setModel(PatchesModel);
}

template <typename TImage>
void TopPatchesWidget<TImage>::SetupScenes()
{
  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(this->SceneBackground);

  this->SourcePatchScene = new QGraphicsScene();
  this->SourcePatchScene->setBackgroundBrush(brush);
  //this->gfxSource->setScene(SourcePatchScene);

}

// template <typename TImage>
// void TopPatchesWidget<TImage>::slot_UpdateSource(const itk::ImageRegion<2>& region)
// {
//   std::cout << "Update source." << std::endl;
// 
//   QImage sourceImage = HelpersQt::GetQImageColor(Image, region);
//   //sourceImage = HelpersQt::FitToGraphicsView(sourceImage, gfxSource);
//   QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));
//   gfxSource->fitInView(item);
// }

#endif
