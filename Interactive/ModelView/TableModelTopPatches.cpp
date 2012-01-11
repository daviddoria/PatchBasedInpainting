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

#include "TableModelTopPatches.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "Helpers/Helpers.h"
#include "HelpersQt.h"

TableModelTopPatches::TableModelTopPatches(QObject * parent, std::vector<InpaintingIterationRecord> const& iterationRecords, DisplayStyle const& displayStyle) :
    QAbstractTableModel(parent), IterationRecords(iterationRecords), IterationToDisplay(0),
    ForwardLookToDisplay(0), PatchDisplaySize(100), NumberOfTopPatchesToDisplay(10), ImageDisplayStyle(displayStyle)
{
}

void TableModelTopPatches::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

Qt::ItemFlags TableModelTopPatches::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

void TableModelTopPatches::SetIterationToDisplay(const unsigned int iteration)
{
  this->IterationToDisplay = iteration;
  Refresh();
}

void TableModelTopPatches::SetForwardLookToDisplay(const unsigned int forwardLook)
{
  this->ForwardLookToDisplay = forwardLook;
  Refresh();
}

int TableModelTopPatches::rowCount(const QModelIndex& parent) const
{
  EnterFunction("TableModelTopPatches::rowCount()");
  if(this->IterationRecords.size() < this->IterationToDisplay ||
    this->IterationRecords.size() == 0 ||
    this->IterationRecords[this->IterationToDisplay].PotentialPairSets.size() == 0 ||
    this->IterationRecords[this->IterationToDisplay].PotentialPairSets.size() < this->ForwardLookToDisplay)
    {
    return 0;
    }
  unsigned int rows = this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay]->GetNumberOfSourcePatches();
  unsigned int numberOfRowsToDisplay = std::min(rows, this->NumberOfTopPatchesToDisplay);
  //std::cout << "Displaying " << numberOfRowsToDisplay << " rows." << std::endl;
  LeaveFunction("TableModelTopPatches::rowCount()");
  return numberOfRowsToDisplay;
}

int TableModelTopPatches::columnCount(const QModelIndex& parent) const
{
  // We have the patch itelf, the location, the id, and then all of the computed values.
  return 3 + this->GetNumberOfDifferences();
}

void TableModelTopPatches::SetNumberOfTopPatchesToDisplay(const unsigned int number)
{
  this->NumberOfTopPatchesToDisplay = number;
}

unsigned int TableModelTopPatches::GetNumberOfDifferences() const
{
  return this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay]->begin()->GetDifferences().GetNumberOfDifferences();
}

QVariant TableModelTopPatches::data(const QModelIndex& index, int role) const
{
  EnterFunction("TableModelTopPatches::data()");
  QVariant returnValue;
  if(role == Qt::DisplayRole && index.row() >= 0)
    {
    const CandidatePairs& currentCandidateSet = *(this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplay]);

    CandidatePairs::ConstIterator pairIterator = currentCandidateSet.begin();
    std::advance(pairIterator, index.row());
    PatchPair patchPair = *pairIterator;
    const Patch* sourcePatch = patchPair.GetSourcePatch();

    switch(index.column())
      {
      case 0:
        {
        FloatVectorImageType* image = dynamic_cast<FloatVectorImageType*>(this->IterationRecords[this->IterationToDisplay].GetImageByName("Image").Image);

        QImage patchImage = HelpersQt::GetQImage<FloatVectorImageType>(image, sourcePatch->GetRegion(), this->ImageDisplayStyle);

        patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);

        returnValue = QPixmap::fromImage(patchImage);
        break;
        }
      case 1:
        {
        returnValue = index.row();
        break;
        }
      case 2:
        {
        returnValue = ITKHelpers::GetIndexString(sourcePatch->GetRegion().GetIndex()).c_str();
        break;
        }
      } // end switch

    if(index.column() > 2)
      {
      unsigned int keyId = index.column() - 3; // There are 3 fixed columns
      returnValue = patchPair.GetDifferences().GetDifferenceNames()[keyId].c_str(); // TODO: this is not safe - there is no reason to believe the underlying map stores the strings in the same order each time.
      }
    } // end if DisplayRole
  LeaveFunction("TopPatchesTableModel::data()");
  return returnValue;
}

QVariant TableModelTopPatches::headerData(int section, Qt::Orientation orientation, int role) const
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
          returnValue = "Id";
          break;
        case 2:
          returnValue = "Location";
          break;
        } // end switch
      if(section > 2)
        {
        // TODO: Fix this
        //PatchPairDifferences::PatchDifferenceTypes key = this->ComputedKeys[section-3];
        //returnValue = PatchPairDifferences::NameOfDifference(key).c_str();
        }
      }// end Horizontal orientation
    } // end DisplayRole

  return returnValue;
}

void TableModelTopPatches::Refresh()
{
  beginResetModel();
  endResetModel();
}

void TableModelTopPatches::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  //std::cout << "TopPatchesTableModel::selectionChanged()" << std::endl;
}
