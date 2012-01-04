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

#ifndef TableModelTopPatches_H
#define TableModelTopPatches_H

// Qt
#include <QAbstractTableModel>
#include <QItemSelection>

// STL
#include <vector>

// Custom
#include "CandidatePairs.h"
#include "DebugOutputs.h"
#include "DisplayStyle.h"
#include "InpaintingIterationRecord.h"

class TableModelTopPatches : public QAbstractTableModel, public DebugOutputs
{
public:
  TableModelTopPatches(QObject * parent, std::vector<InpaintingIterationRecord> const& iterationRecords, DisplayStyle const& displayStyle);

  int rowCount(const QModelIndex& parent) const;
  int columnCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  void SetIterationToDisplay(const unsigned int);
  void SetForwardLookToDisplay(const unsigned int);
  void SetNumberOfTopPatchesToDisplay(const unsigned int);

  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void Refresh();

  void SetPatchDisplaySize(const unsigned int value);

private:

  // The outer vector is the iteration, and the inner vector is the look ahead patch.
  std::vector<InpaintingIterationRecord> const& IterationRecords;

  unsigned int GetNumberOfDifferences() const;

  unsigned int IterationToDisplay;
  unsigned int ForwardLookToDisplay;

  unsigned int PatchDisplaySize;

  unsigned int NumberOfTopPatchesToDisplay;

  DisplayStyle const& ImageDisplayStyle;

};

#endif
