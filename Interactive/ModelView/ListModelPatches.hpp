#include "ListModelPatches.h" // Appease the syntax parser

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "Helpers/Helpers.h"
#include "Interactive/HelpersQt.h"

template <typename TImage>
ListModelPatches<TImage>::ListModelPatches(TImage* const image, QObject * const parent) :
    QAbstractTableModel(parent), PatchDisplaySize(100), NumberOfTopPatchesToDisplay(10)
{
}

template <typename TImage>
void ListModelPatches<TImage>::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

template <typename TImage>
Qt::ItemFlags ListModelPatches<TImage>::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

template <typename TImage>
int ListModelPatches<TImage>::rowCount(const QModelIndex& parent) const
{
  return this->NumberOfTopPatchesToDisplay;
}

template <typename TImage>
int ListModelPatches<TImage>::columnCount(const QModelIndex& parent) const
{
  return 1;
}

template <typename TImage>
void ListModelPatches<TImage>::SetNumberOfTopPatchesToDisplay(const unsigned int number)
{
  this->NumberOfTopPatchesToDisplay = number;
}

template <typename TImage>
QVariant ListModelPatches<TImage>::data(const QModelIndex& index, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole && index.row() >= 0)
    {
    QImage patchImage = HelpersQt::GetQImage<FloatVectorImageType>(this->Image, sourcePatch->GetRegion(), this->ImageDisplayStyle);

    patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);

    returnValue = QPixmap::fromImage(patchImage);
    } // end if DisplayRole

  return returnValue;
}

template <typename TImage>
QVariant ListModelPatches<TImage>::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole)
    {
    if(orientation == Qt::Horizontal)
      {
      if(section == 0)
        {
          returnValue = "Patch";
        }
      }// end Horizontal orientation
    } // end DisplayRole

  return returnValue;
}

template <typename TImage>
void ListModelPatches<TImage>::Refresh()
{
  beginResetModel();
  endResetModel();
}

template <typename TImage>
void ListModelPatches<TImage>::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  //std::cout << "TopPatchesTableModel::selectionChanged()" << std::endl;
}
