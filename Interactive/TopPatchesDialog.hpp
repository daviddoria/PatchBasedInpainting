#ifndef TopPatchesDialog_HPP
#define TopPatchesDialog_HPP

#include "TopPatchesDialog.h" // Appease syntax parser

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

template <typename TImage>
TopPatchesDialog<TImage>::TopPatchesDialog(TImage* const image, Mask* const mask, const unsigned int patchHalfWidth) :
Image(image), MaskImage(mask), SelectedItem(-1), PatchHalfWidth(patchHalfWidth)
{
  this->setupUi(this);

  MaskedQueryPatchItem = new QGraphicsPixmapItem;

  this->QueryPatchScene = new QGraphicsScene();
  //this->QueryPatchScene->setBackgroundBrush(brush);
  this->gfxQueryPatch->setScene(QueryPatchScene);

  PatchesModel = new ListModelPatches<TImage>(image, patchHalfWidth);
  this->listView->setModel(PatchesModel);

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;

  this->listView->setItemDelegate(pixmapDelegate);

  connect(this->listView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slot_Selected(const QModelIndex&)));
}

template <typename TImage>
void TopPatchesDialog<TImage>::SetSourceNodes(const std::vector<Node>& nodes)
{
  this->PatchesModel->SetNodes(nodes);
}

template <typename TImage>
void TopPatchesDialog<TImage>::SetQueryNode(const Node& queryNode)
{
  this->QueryNode = queryNode;

  itk::Index<2> queryIndex = ITKHelpers::CreateIndex(queryNode);
  itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex, PatchHalfWidth);
  QImage maskedQueryPatch = HelpersQt::GetQImageMasked(Image, MaskImage, queryRegion);
  MaskedQueryPatchItem = this->QueryPatchScene->addPixmap(QPixmap::fromImage(maskedQueryPatch));

  // We do this here because we could potentially call SetQueryNode after the widget is constructed as well.
  gfxQueryPatch->fitInView(MaskedQueryPatchItem);
}

template <typename TImage>
void TopPatchesDialog<TImage>::slot_Selected(const QModelIndex& selected)
{
  SelectedItem = selected.row();
  // std::cout << "Selected " << selected.row() << std::endl;
  accept();
}

template <typename TImage>
unsigned int TopPatchesDialog<TImage>::GetSelectedItem()
{
  return SelectedItem;
}

template <typename TImage>
void TopPatchesDialog<TImage>::showEvent(QShowEvent* event)
{
  SelectedItem = -1;
  // We do this here because we will usually call SetQueryNode before the widget is constructed (i.e. before exec() is called).
  gfxQueryPatch->fitInView(MaskedQueryPatchItem);
}

#endif
