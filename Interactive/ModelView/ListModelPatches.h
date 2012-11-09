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
#include <QAbstractListModel>
#include <QItemSelection>

// ITK
#include "itkIndex.h"
#include "itkImageRegion.h"

// STL
#include <vector>

// Custom
#include "Node.h"

template <typename TImage>
class ListModelPatches : public QAbstractListModel
{
public:
  ListModelPatches(const TImage* const image, const unsigned int patchHalfWidth,
                   QObject * const parent = 0);

  int rowCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  void Refresh();

  // void SetPatchDisplaySize(const unsigned int sideLength);
  void SetRegions(const std::vector<itk::ImageRegion<2> >& regions);

  void SetNodes(const std::vector<Node>& nodes);

  void SetRowHeight(const unsigned int rowHeight);

  void SetNumberOfTopPatchesToDisplay(const unsigned int);

private:

  /** This is the image that the displayed patches are created from. */
  const TImage* Image;

  /** This is an ordered list of the nodes at which to display the corresponding patches. */
  // std::vector<itk::Index<2> > Nodes;
  std::vector<itk::ImageRegion<2> > Regions;

  /** This is the side length, in pixels, of the display of the patches (they will be scaled to this size from whatever size they actually are in the algorithm). */
  // unsigned int PatchDisplaySize; // This should be determined by the width of the view

  /** The number of patches to display in the view. Originally this is determined by the length of the Regions vector, but it can be modified. */
  mutable unsigned int NumberOfTopPatchesToDisplay;

  unsigned int RowHeight = 50;

  unsigned int PatchHalfWidth;
};

#include "ListModelPatches.hpp"

#endif
