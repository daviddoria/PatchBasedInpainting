#include "ForwardLookTableModel.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "Helpers.h"

ForwardLookTableModel::ForwardLookTableModel(std::vector<std::vector<CandidatePairs> >& allCandidatePairs) : QAbstractTableModel(), AllCandidatePairs(allCandidatePairs)
{
  this->IterationToDisplay = 0;
  
  this->PatchDisplaySize = 100;
}

void ForwardLookTableModel::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

Qt::ItemFlags ForwardLookTableModel::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

void ForwardLookTableModel::SetIterationToDisplay(const unsigned int iteration)
{
  this->IterationToDisplay = iteration;
  Refresh();
}

void ForwardLookTableModel::SetImage(FloatVectorImageType::Pointer image)
{
  this->Image = image;
}

int ForwardLookTableModel::rowCount(const QModelIndex& parent) const
{
  if(this->AllCandidatePairs.size() < this->IterationToDisplay || 
    this->AllCandidatePairs.size() == 0)
    {
    return 0;
    }
  return this->AllCandidatePairs[this->IterationToDisplay].size();
}

int ForwardLookTableModel::columnCount(const QModelIndex& parent) const
{
  return 3;
}

QVariant ForwardLookTableModel::data(const QModelIndex& index, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole && index.row() >= 0)
    {
    const CandidatePairs& currentCandidateSet = this->AllCandidatePairs[this->IterationToDisplay][index.row()];
    const Patch& currentForwardLookPatch = currentCandidateSet.TargetPatch;
    switch(index.column())
      {
      case 0:
	{
	// Display the target patch in the table
	QImage patchImage = Helpers::GetQImageColor<FloatVectorImageType>(this->Image, currentForwardLookPatch.Region);
	
	patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);
    
	returnValue = QPixmap::fromImage(patchImage);
	break;
	}
      case 1:
	{
	// Display priority in the table
	returnValue = currentCandidateSet.Priority;
	break;
	}
      case 2:
	{
	// Display the target patch location in the table
	std::stringstream ssLocation;
	ssLocation << "(" << currentForwardLookPatch.Region.GetIndex()[0] << ", " << currentForwardLookPatch.Region.GetIndex()[1] << ")";
	returnValue = ssLocation.str().c_str();
	break;
	}
      } // end switch
    } // end if DisplayRole
    
  return returnValue;
}

QVariant ForwardLookTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole)
    {
    if(orientation == Qt::Horizontal)
      {
      switch(section)
	{
	case 0:
	  returnValue = "Patch";
	  break;
	case 1:
	  returnValue = "Priority";
	  break;
	case 2:
	  returnValue = "Location";
	  break;
	} // end switch
      }// end Horizontal orientation
    } // end DisplayRole
  
  return returnValue;
}

void ForwardLookTableModel::Refresh()
{
  //std::cout << "ForwardLookTableModel::Refresh(): Displaying iteration: " << this->IterationToDisplay << std::endl;
  beginResetModel();
  endResetModel();
}

void ForwardLookTableModel::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  //std::cout << "ForwardLookTableModel::selectionChanged()" << std::endl;
}
