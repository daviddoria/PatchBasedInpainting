#include "ListModelPatches.h" // Appease the syntax parser

// Qt
#include <QPixmap>

// Custom
#include "Helpers/Helpers.h"
#include "QtHelpers/QtHelpers.h"
#include "QtHelpers/ITKQtHelpers.h"
#include "ITKHelpers/ITKHelpers.h"

template <typename TImage>
ListModelPatches<TImage>::ListModelPatches(TImage* const image, const unsigned int patchHalfWidth, QObject * const parent) :
    QAbstractListModel(parent), Image(image), RowHeight(50), PatchHalfWidth(patchHalfWidth)
{
}

template <typename TImage>
Qt::ItemFlags ListModelPatches<TImage>::flags(const QModelIndex& index) const
{
  //return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

template <typename TImage>
int ListModelPatches<TImage>::rowCount(const QModelIndex& parent) const
{
  //std::cout << "rowCount: number of regions " << this->Regions.size() << std::endl;;
  return this->Regions.size();
}

template <typename TImage>
void ListModelPatches<TImage>::SetNumberOfTopPatchesToDisplay(const unsigned int number)
{
  this->NumberOfTopPatchesToDisplay = number;
}

template <typename TImage>
void ListModelPatches<TImage>::SetRegions(const std::vector<itk::ImageRegion<2> >& regions)
{
  this->Regions = regions;
  Refresh();
}

template <typename TImage>
void ListModelPatches<TImage>::SetNodes(const std::vector<Node>& nodes)
{
  // std::cout << "ListModelPatches::SetNodes called with " << nodes.size() << " nodes." << std::endl;
  this->Regions.clear();
  for(std::vector<Node>::const_iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
    {
    itk::Index<2> index = ITKHelpers::CreateIndex(*iter);
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->PatchHalfWidth);
    this->Regions.push_back(region);
    }
  // std::cout << "SetNodes Refresh()" << std::endl;
  Refresh();
}

template <typename TImage>
QVariant ListModelPatches<TImage>::data(const QModelIndex& index, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole && index.row() >= 0 && index.row() < static_cast<int>(this->Regions.size()))
    {
    //std::cout << "ListModelPatches::data requesting index " << index.row() << std::endl;
    //std::cout << "ListModelPatches::data requesting region " << this->Regions[index.row()] << std::endl;
    QImage patchImage;
    try
    {
      if(!this->Image->GetLargestPossibleRegion().IsInside(this->Regions[index.row()]))
      {
        // If one patch is found to be invalid, the rest are definitely invalid also.
        //SetNumberOfTopPatchesToDisplay(index.row());
        NumberOfTopPatchesToDisplay = index.row();
        
        return QVariant();
      }
      patchImage = ITKQtHelpers::GetQImageColor(this->Image, this->Regions[index.row()]);
    }
    catch (...)
    {
      std::cout << "ListModelPatches::data something is wrong with region " << this->Regions[index.row()] << std::endl;
    }
    // patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);

    returnValue = QPixmap::fromImage(patchImage);
    } // end if DisplayRole
  else if(role == Qt::SizeHintRole) // This sets the row height in the QListView
    {
    QSize size;
    size.setHeight(RowHeight);
    return size;
    }

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
void ListModelPatches<TImage>::SetRowHeight(const unsigned int rowHeight)
{
  this->RowHeight = rowHeight;
  Refresh();
}
