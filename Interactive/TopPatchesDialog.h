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

#ifndef TopPatchesDialog_H
#define TopPatchesDialog_H

#include "ui_TopPatchesDialog.h"

// Qt
#include <QDialog>
#include <QObject>

// Custom
#include "ImageProcessing/Mask.h"
#include "Node.h"
#include "Interactive/ModelView/ListModelPatches.h"

/** This class is necessary because a class template cannot have the Q_OBJECT macro directly. */
class TopPatchesDialogParent : public QDialog, public Ui::TopPatchesDialog
{
Q_OBJECT

public slots:

  virtual void SetSourceNodes(const std::vector<Node>& sourceNodes) = 0;
  virtual void SetQueryNode(const Node& queryNode) = 0;
  virtual void slot_Selected(const QModelIndex & index) = 0;
};

/** This class displays a set of patches in an ordered list. */
template <typename TImage>
class TopPatchesDialog : public TopPatchesDialogParent
{
private:
  /** The image that will be displayed, and the from which the patches will be extracted before being displayed. */
  TImage* Image;
  Mask* MaskImage;

  Node QueryNode;

  unsigned int SelectedItem;
  unsigned int PatchHalfWidth;

  QGraphicsScene* QueryPatchScene;
  QGraphicsPixmapItem* MaskedQueryPatchItem;
  
  // The color to use as the background of the QGraphicsScenes
  QColor SceneBackground;

  ListModelPatches<TImage>* PatchesModel;

  void showEvent(QShowEvent* event);

public:
  /** Constructor */
  TopPatchesDialog(TImage* const image, Mask* const mask, const unsigned int patchHalfWidth);

  /** Set the source nodes from which the user can choose. */
  void SetSourceNodes(const std::vector<Node>& nodes);

  /** Set the query node that the user will choose the best match to. */
  void SetQueryNode(const Node& node);

  /** Catch the signal that the ListView emits when it is clicked. */
  void slot_Selected(const QModelIndex & index);

  /** Get the id of the node that user selected. */
  unsigned int GetSelectedItem();

  //void on_btnRefresh_clicked();
};

#include "TopPatchesDialog.hpp"

#endif // TopPatchesDialog_H
