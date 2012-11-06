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
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "QtHelpers/QtHelpers.h"
#include "InteractorStyleImageWithDrag.h"
#include "Mask/Mask.h"
#include "Interactive/Delegates/PixmapDelegate.h"
#include "Interactive/ManualPatchSelectionDialog.h"
#include "Utilities/PatchHelpers.h"

template <typename TImage>
TopPatchesDialog<TImage>::TopPatchesDialog(TImage* const image, Mask* const mask,
                                           const unsigned int patchHalfWidth, QWidget* parent) :
  TopPatchesDialogParent(parent), Image(image), MaskImage(mask),
  ValidSelection(false), PatchHalfWidth(patchHalfWidth)
{
  this->setupUi(this);

//   if(image->GetNumberOfComponentsPerPixel() == 3)
//   {
//     // assume the image is RGB, and use it directly
//     ITKHelpers::DeepCopy(image, this->Image);
//   }
  this->MaskedQueryPatchItem = new QGraphicsPixmapItem;
  this->QueryPatchScene = new QGraphicsScene();
  this->gfxQueryPatch->setScene(this->QueryPatchScene);

  this->ProposedPatchItem = new QGraphicsPixmapItem;
  this->ProposedPatchScene = new QGraphicsScene();
  this->gfxProposedPatch->setScene(this->ProposedPatchScene);

  this->PatchesModel = new ListModelPatches<TImage>(image, patchHalfWidth);
  // listView is a GUI object
  this->listView->setModel(this->PatchesModel);

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;

  this->listView->setItemDelegate(pixmapDelegate);

  connect(this->listView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slot_SingleClicked(const QModelIndex&)));
  
  connect(this->listView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(slot_DoubleClicked(const QModelIndex&)));
}

//template <typename TImage>
//TopPatchesDialog<TImage>::~TopPatchesDialog()
//{
//  std::cout << "TopPatchesDialog was used " << this->NumberOfUses << " times." << std::endl;
//}

template <typename TImage>
void TopPatchesDialog<TImage>::SetSourceNodes(const std::vector<Node>& nodes)
{
  this->Nodes = nodes;

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
  itk::ImageRegion<2> queryRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex, PatchHalfWidth);

  typename TImage::Pointer regionImage = TImage::New();
  ITKHelpers::ExtractRegion(this->Image, queryRegion,
                            regionImage.GetPointer());

  Mask::Pointer regionMask = Mask::New();
  ITKHelpers::ExtractRegion(this->MaskImage, queryRegion,
                            regionMask.GetPointer());
  regionMask->CopyInformationFrom(this->MaskImage);
  std::cout << "There are " << regionMask->CountHolePixels() << " hole pixels." << std::endl;

  typename TImage::PixelType zeroPixel(3);
  zeroPixel = itk::NumericTraits<typename TImage::PixelType>::ZeroValue(zeroPixel);
//  regionMask->ApplyToImage(regionImage.GetPointer(), zeroPixel);

  // We must now refer to the regionImage as a new, standalone image
  // (use it's LargestPossibleRegion instead of queryRegion)
  QImage maskedQueryPatch =
//      ITKQtHelpers::GetQImageColor(regionImage.GetPointer(), queryRegion);
      ITKQtHelpers::GetQImageColor(regionImage.GetPointer(), regionImage->GetLargestPossibleRegion());
  this->MaskedQueryPatchItem =
      this->QueryPatchScene->addPixmap(QPixmap::fromImage(maskedQueryPatch));

  // We do this here because we could potentially call SetQueryNode after
  // the widget is constructed as well.
  gfxQueryPatch->fitInView(MaskedQueryPatchItem);
}

template <typename TImage>
void TopPatchesDialog<TImage>::slot_DoubleClicked(const QModelIndex& selected)
{
  this->SelectedNode = this->Nodes[selected.row()];
  this->ValidSelection = true;
  //std::cout << "Selected " << selected.row() << std::endl;
  std::cout << "SelectedNode : " << this->SelectedNode[0] << " "
            << this->SelectedNode[1] << std::endl;
  accept();
}

template <typename TImage>
void TopPatchesDialog<TImage>::slot_SingleClicked(const QModelIndex& selected)
{
  itk::Index<2> queryIndex = ITKHelpers::CreateIndex(this->QueryNode);

  itk::ImageRegion<2> queryRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex, this->PatchHalfWidth);

  itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(this->Nodes[selected.row()]);

  itk::ImageRegion<2> sourceRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, this->PatchHalfWidth);

  emit signal_SelectedRegion(sourceRegion);

  {
  Mask::Pointer regionMask = Mask::New();
  ITKHelpers::ExtractRegion(this->MaskImage, sourceRegion,
                            regionMask.GetPointer());
  regionMask->CopyInformationFrom(this->MaskImage);
  std::cout << "There are " << regionMask->CountHolePixels() << " hole pixels in the source region." << std::endl;
  }

  QImage proposedPatch =
      PatchHelpers::GetQImageCombinedPatch(this->Image, sourceRegion, queryRegion, this->MaskImage);

  this->ProposedPatchItem =
      this->ProposedPatchScene->addPixmap(QPixmap::fromImage(proposedPatch));

  gfxProposedPatch->fitInView(this->ProposedPatchItem);

  this->SelectedIndex = selected;
}

template <typename TImage>
Node TopPatchesDialog<TImage>::GetSelectedNode()
{
  return this->SelectedNode;
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
  itk::Index<2> queryIndex = ITKHelpers::CreateIndex(this->QueryNode);
  itk::ImageRegion<2> queryRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex, this->PatchHalfWidth);
  ManualPatchSelectionDialog<TImage> manualPatchSelectionDialog(this->Image, this->MaskImage, queryRegion);
  manualPatchSelectionDialog.exec();

  if(manualPatchSelectionDialog.result() == QDialog::Rejected)
  {
    std::cout << "Did not choose patch manually." << std::endl;
  }
  else if(manualPatchSelectionDialog.result() == QDialog::Accepted)
  {
    std::cout << "Chose patch manually." << std::endl;
    this->SelectedNode = manualPatchSelectionDialog.GetSelectedNode();
    this->ValidSelection = true;
    std::cout << "SelectedNode : " << this->SelectedNode[0] << " "
              << this->SelectedNode[1] << std::endl;

    // Close the dialog
    accept();
  }
}

template <typename TImage>
bool TopPatchesDialog<TImage>::IsSelectionValid() const
{
  return this->ValidSelection;
}

template <typename TImage>
void TopPatchesDialog<TImage>::on_btnSavePair_clicked()
{
  // Save the query patch
  itk::Index<2> queryIndex = ITKHelpers::CreateIndex(QueryNode);
  itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex, PatchHalfWidth);
  ITKHelpers::WriteRegionAsRGBImage(this->Image, queryRegion, "query.png");

  // Save the source patch
  // Can't do this - it is protected. Instead we have created a member variable 'SelectedIndex'.
  //QModelIndex selectedIndex = this->listView->selectedIndexes().begin(); 
  itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(Nodes[this->SelectedIndex.row()]);
  itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, PatchHalfWidth);
  ITKHelpers::WriteRegionAsRGBImage(this->Image, sourceRegion, "source.png");
}

template <typename TImage>
void TopPatchesDialog<TImage>::PositionNextToParent()
{
  if(this->parentWidget())
  {
    this->move(this->parentWidget()->pos().x() + this->parentWidget()->width(),
               this->parentWidget()->pos().y());
  }
}

#endif
