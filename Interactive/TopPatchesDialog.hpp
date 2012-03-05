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
#include "Interactive/ManualPatchSelectionDialog.h"

template <typename TImage>
TopPatchesDialog<TImage>::TopPatchesDialog(TImage* const image, Mask* const mask, const unsigned int patchHalfWidth) :
Image(image), MaskImage(mask), ValidSelection(false), PatchHalfWidth(patchHalfWidth)
{
  this->setupUi(this);

  MaskedQueryPatchItem = new QGraphicsPixmapItem;
  this->QueryPatchScene = new QGraphicsScene();
  this->gfxQueryPatch->setScene(QueryPatchScene);

  ProposedPatchItem = new QGraphicsPixmapItem;
  this->ProposedPatchScene = new QGraphicsScene();
  this->gfxProposedPatch->setScene(ProposedPatchScene);

  PatchesModel = new ListModelPatches<TImage>(image, patchHalfWidth);
  this->listView->setModel(PatchesModel);

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;

  this->listView->setItemDelegate(pixmapDelegate);

  connect(this->listView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slot_SingleClicked(const QModelIndex&)));
  
  connect(this->listView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(slot_DoubleClicked(const QModelIndex&)));
}

template <typename TImage>
void TopPatchesDialog<TImage>::SetSourceNodes(const std::vector<Node>& nodes)
{
  Nodes = nodes;
  
  this->PatchesModel->SetNodes(nodes);
}

template <typename TImage>
template <typename TNode>
void TopPatchesDialog<TImage>::SetSourceNodes(const std::vector<TNode>& sourceNodes)
{
  std::vector<Node> nodes;
  for(unsigned int i = 0; i < sourceNodes.size(); ++i)
    {
    Node node = Helpers::ConvertFrom<Node, TNode>(sourceNodes[i]);
    nodes.push_back(node);
    }
    
  SetSourceNodes(nodes);
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
void TopPatchesDialog<TImage>::slot_DoubleClicked(const QModelIndex& selected)
{
  SelectedNode = Nodes[selected.row()];
  ValidSelection = true;
  //std::cout << "Selected " << selected.row() << std::endl;
  accept();
}

template <typename TImage>
void TopPatchesDialog<TImage>::slot_SingleClicked(const QModelIndex& selected)
{
  itk::Index<2> queryIndex = ITKHelpers::CreateIndex(QueryNode);
  itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex, PatchHalfWidth);

  itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(Nodes[selected.row()]);
  itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, PatchHalfWidth);
  
  QImage proposedPatch = HelpersQt::GetQImageCombinedPatch(Image, sourceRegion, queryRegion, MaskImage);
  ProposedPatchItem = this->ProposedPatchScene->addPixmap(QPixmap::fromImage(proposedPatch));
  gfxProposedPatch->fitInView(ProposedPatchItem);
}

template <typename TImage>
Node TopPatchesDialog<TImage>::GetSelectedNode()
{
  return SelectedNode;
}

template <typename TImage>
void TopPatchesDialog<TImage>::showEvent(QShowEvent* event)
{
  ValidSelection = false;
  // We do this here because we will usually call SetQueryNode before the widget is constructed (i.e. before exec() is called).
  gfxQueryPatch->fitInView(MaskedQueryPatchItem);

  // Make sure the list is scrolled to the top
  QModelIndex index = this->PatchesModel->index(0,0);
  this->listView->scrollTo(index);

  // Setup the proposed patch (if the best patch were to be selected)
  slot_SingleClicked(index);
  gfxProposedPatch->fitInView(ProposedPatchItem);

}

template <typename TImage>
void TopPatchesDialog<TImage>::on_btnSelectManually_clicked()
{
  itk::Index<2> queryIndex = ITKHelpers::CreateIndex(QueryNode);
  itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex, PatchHalfWidth);
  ManualPatchSelectionDialog<TImage> manualPatchSelectionDialog(Image, MaskImage, queryRegion);
  manualPatchSelectionDialog.exec();

  if(manualPatchSelectionDialog.result() == QDialog::Rejected)
  {
    std::cout << "Did not choose patch manually." << std::endl;
  }
  else if(manualPatchSelectionDialog.result() == QDialog::Accepted)
  {
    std::cout << "Chose patch manually." << std::endl;
    SelectedNode = manualPatchSelectionDialog.GetSelectedNode();
    ValidSelection = true;
    std::cout << "SelectedNode : " << SelectedNode[0] << " " << SelectedNode[1] << std::endl;
    accept();
  }
}

template <typename TImage>
bool TopPatchesDialog<TImage>::IsSelectionValid() const
{
  return ValidSelection;
}

#endif
