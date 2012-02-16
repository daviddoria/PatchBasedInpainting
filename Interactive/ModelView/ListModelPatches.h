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

#ifndef ListModelPatches_H
#define ListModelPatches_H

// Qt
#include <QAbstractTableModel>
#include <QItemSelection>

// STL
#include <vector>

template <typename TImage>
class ListModelPatches : public QAbstractTableModel
{
public:
  ListModelPatches(QObject * parent);

  int rowCount(const QModelIndex& parent) const;
  int columnCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  void SetNumberOfTopPatchesToDisplay(const unsigned int);

  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void Refresh();

  void SetPatchDisplaySize(const unsigned int sideLength);

private:

  /** This is the image that the displayed patches are created from. */
  TImage* Image;

  /** This is the side length, in pixels, of the display of the patches (they will be scaled to this size from whatever size they actually are in the algorithm). */
  unsigned int PatchDisplaySize;

  /** The number of patches to display in the view. */
  unsigned int NumberOfTopPatchesToDisplay;

};

#endif
