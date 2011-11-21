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

#include "ForwardLookTableModel.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "Helpers.h"
#include "HelpersQt.h"

ForwardLookTableModel::ForwardLookTableModel(std::vector<InpaintingIterationRecord>& iterationRecords, DisplayStyle& style) :
    QAbstractTableModel(), IterationRecords(iterationRecords), IterationToDisplay(0), PatchDisplaySize(100), ImageDisplayStyle(style)
{
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

int ForwardLookTableModel::rowCount(const QModelIndex& parent) const
{
  EnterFunction("ForwardLookTableModel::rowCount()");
  if(this->IterationRecords.size() < this->IterationToDisplay || 
    this->IterationRecords.size() == 0)
    {
    return 0;
    }
  unsigned int rows = this->IterationRecords[this->IterationToDisplay].PotentialPairSets.size();
  LeaveFunction("ForwardLookTableModel::rowCount()");
  return rows;
}

int ForwardLookTableModel::columnCount(const QModelIndex& parent) const
{
  return 3;
}

QVariant ForwardLookTableModel::data(const QModelIndex& index, int role) const
{
  EnterFunction("ForwardLookTableModel::data()");
  QVariant returnValue;
  if(role == Qt::DisplayRole && index.row() >= 0)
    {
    const CandidatePairs& currentCandidateSet = this->IterationRecords[this->IterationToDisplay].PotentialPairSets[index.row()];
    const Patch& currentForwardLookPatch = currentCandidateSet.TargetPatch;
    switch(index.column())
      {
      case 0:
	{
	// Display the target patch in the table
	QImage patchImage = HelpersQt::GetQImage<FloatVectorImageType>(this->IterationRecords[this->IterationToDisplay].Image,
                                                                       currentForwardLookPatch.Region, this->ImageDisplayStyle);
	
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
  LeaveFunction("ForwardLookTableModel::data()");
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
