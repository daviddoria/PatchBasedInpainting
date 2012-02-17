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
#include "Interactive/Delegates/PixmapDelegate.h"

Q_DECLARE_METATYPE(std::vector<Node>)

template <typename TImage>
TopPatchesWidget<TImage>::TopPatchesWidget(TImage* const image, const unsigned int patchHalfWidth) : Image(image)
{
  //qRegisterMetaType<itk::ImageRegion<2> >("itkImageRegion");
  qRegisterMetaType<std::vector<Node> >("VectorOfNodes");

  this->setupUi(this);

  SetupScenes();

  PatchesModel = new ListModelPatches<TImage>(image, patchHalfWidth);
  this->listView->setModel(PatchesModel);

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;
  //this->listView->setItemDelegateForColumn(0, pixmapDelegate);
  this->listView->setItemDelegate(pixmapDelegate);
}

template <typename TImage>
void TopPatchesWidget<TImage>::SetNodes(const std::vector<Node>& nodes)
{
  std::cout << "Forwarding nodes to the mode..." << std::endl;
  this->PatchesModel->SetNodes(nodes);
}

// template <typename TImage>
// ListModelPatches<TImage>* TopPatchesWidget<TImage>::GetPatchesModel()
// {
//   return this->PatchesModel;
// }

// template <typename TImage>
// void TopPatchesWidget<TImage>::on_btnRefresh_clicked()
// {
//   this->PatchesModel->Refresh();;
// }

template <typename TImage>
void TopPatchesWidget<TImage>::slot_Refresh()
{
  this->PatchesModel->Refresh();
}

template <typename TImage>
void TopPatchesWidget<TImage>::SetupScenes()
{

}

#endif
