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

#include "ListModelPatches.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "Helpers/Helpers.h"
#include "Interactive/HelpersQt.h"

ListModelPatches::ListModelPatches(QObject * parent) :
    QAbstractTableModel(parent), PatchDisplaySize(100), NumberOfTopPatchesToDisplay(10)
{
}

void ListModelPatches::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

Qt::ItemFlags ListModelPatches::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

int ListModelPatches::rowCount(const QModelIndex& parent) const
{
  return this->NumberOfTopPatchesToDisplay;
}

int ListModelPatches::columnCount(const QModelIndex& parent) const
{
  return 1;
}

void ListModelPatches::SetNumberOfTopPatchesToDisplay(const unsigned int number)
{
  this->NumberOfTopPatchesToDisplay = number;
}

QVariant ListModelPatches::data(const QModelIndex& index, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole && index.row() >= 0)
    {
    QImage patchImage = HelpersQt::GetQImage<FloatVectorImageType>(image, sourcePatch->GetRegion(), this->ImageDisplayStyle);

    patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);

    returnValue = QPixmap::fromImage(patchImage);
    } // end if DisplayRole

  return returnValue;
}

QVariant ListModelPatches::headerData(int section, Qt::Orientation orientation, int role) const
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

void ListModelPatches::Refresh()
{
  beginResetModel();
  endResetModel();
}

void ListModelPatches::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  //std::cout << "TopPatchesTableModel::selectionChanged()" << std::endl;
}
