/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "TopPatchesTableModel.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "Helpers.h"
#include "HelpersQt.h"

TopPatchesTableModel::TopPatchesTableModel(std::vector<std::vector<CandidatePairs> >& allCandidatePairs, DisplayStyle& displayStyle) :
    QAbstractTableModel(), AllCandidatePairs(allCandidatePairs), ImageDisplayStyle(displayStyle), IterationToDisplay(0),
    ForwardLookToDisplay(0), PatchDisplaySize(100)
{
}

void TopPatchesTableModel::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

Qt::ItemFlags TopPatchesTableModel::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

void TopPatchesTableModel::SetIterationToDisplay(const unsigned int iteration)
{
  this->IterationToDisplay = iteration;
  Refresh();
}

void TopPatchesTableModel::SetForwardLookToDisplay(const unsigned int forwardLook)
{
  this->ForwardLookToDisplay = forwardLook;
  Refresh();
}

void TopPatchesTableModel::SetImage(FloatVectorImageType::Pointer image)
{
  this->Image = image;
}

int TopPatchesTableModel::rowCount(const QModelIndex& parent) const
{
  if(this->AllCandidatePairs.size() < this->IterationToDisplay || 
    this->AllCandidatePairs.size() == 0)
    {
    return 0;
    }
  return this->AllCandidatePairs[this->IterationToDisplay][this->ForwardLookToDisplay].size();
}

int TopPatchesTableModel::columnCount(const QModelIndex& parent) const
{
  return 4;
}

QVariant TopPatchesTableModel::data(const QModelIndex& index, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole && index.row() >= 0)
    {
    const CandidatePairs& currentCandidateSet = this->AllCandidatePairs[this->IterationToDisplay][this->ForwardLookToDisplay];
    
    switch(index.column())
      {
      case 0:
	{
	QImage patchImage = HelpersQt::GetQImage<FloatVectorImageType>(this->Image, currentCandidateSet[index.row()].SourcePatch.Region, this->ImageDisplayStyle);
	
	patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);
    
	returnValue = QPixmap::fromImage(patchImage);
	break;
	}
      case 1:
	{
	returnValue = currentCandidateSet[index.row()].GetAverageAbsoluteDifference();
	break;
	}
      case 2:
	{
	returnValue = index.row();
	break;
	}
      case 3:
	{
	std::stringstream ss;
	ss << "(" << currentCandidateSet[index.row()].SourcePatch.Region.GetIndex()[0] << ", " << currentCandidateSet[index.row()].SourcePatch.Region.GetIndex()[1] << ")";
	returnValue = ss.str().c_str();
	break;
	}
      } // end switch
    } // end if DisplayRole
    
  return returnValue;
}

QVariant TopPatchesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
	  returnValue = "Av. Abs. Diff.";
	  break;
	case 2:
	  returnValue = "Id";
	  break;
	case 3:
	  returnValue = "Location";
	  break;
	} // end switch
      }// end Horizontal orientation
    } // end DisplayRole
  
  return returnValue;
}

void TopPatchesTableModel::Refresh()
{
  //std::cout << "TopPatchesTableModel::Refresh(): Displaying iteration: " << this->IterationToDisplay << std::endl;
  beginResetModel();
  endResetModel();
}

void TopPatchesTableModel::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  //std::cout << "TopPatchesTableModel::selectionChanged()" << std::endl;
}
