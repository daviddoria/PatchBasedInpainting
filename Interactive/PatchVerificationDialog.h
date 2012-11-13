/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef PatchVerificationDialog_H
#define PatchVerificationDialog_H

#include "ui_PatchVerificationDialog.h"

// Qt
#include <QDialog>
#include <QObject>

// Custom
#include "Node.h"
#include "Interactive/ModelView/ListModelPatches.h"

// Submodules
#include <Mask/Mask.h>

/** This class is necessary because a class template cannot have the Q_OBJECT macro directly. */
class PatchVerificationDialogParent : public QDialog, public Ui::PatchVerificationDialog
{
Q_OBJECT

public:
  /** Constructor. */
  PatchVerificationDialogParent(QWidget* parent = nullptr) : QDialog(parent) {}

signals:
  void signal_SelectedRegion(const itk::ImageRegion<2>&) const; // We treat this as virtual, but Qt says "Signals cannot be declared virtual."

public slots:

  /** Set the source nodes. Ideally this would be templated on the node type, but since it is a slot it cannot be templated. */
  virtual void SetSourceNode(const Node& sourceNode) = 0;

  /** Set the query node. */
  virtual void SetQueryNode(const Node& queryNode) = 0;

  /** The slot to handle when the btnAccept button is clicked. */
  virtual void on_btnAccept_clicked() = 0;

  /** Update the source and target region outlines and display the source and target patches.*/
  virtual void slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion,
                                 const itk::ImageRegion<2>& targetRegion) = 0;

  /** The slot to handle when the btnSelectManually button is clicked. */
  virtual void on_btnSelectManually_clicked() = 0;
};

/** This class displays a set of patches in an ordered list. */
template <typename TImage>
class PatchVerificationDialog : public PatchVerificationDialogParent
{
private:
  /** The image that will be displayed, and the from which the patches will be extracted before being displayed. */
  const TImage* Image;

  /** The mask to use. */
  const Mask* MaskImage;

  /** The query node. */
  Node SourceNode;

  /** The query node. */
  Node QueryNode;

  /** Store the selected node. */
  Node SelectedNode;

  /** The half-width of the patch. */
  unsigned int PatchHalfWidth;

  /** The scene for the query patch.*/
  QGraphicsScene* QueryPatchScene;

  /** The item for the query patch.*/
  QGraphicsPixmapItem* MaskedQueryPatchItem;

  /** The scene for the proposed patch.*/
  QGraphicsScene* SourcePatchScene;

  /** The item for the proposed patch.*/
  QGraphicsPixmapItem* SourcePatchItem;

  /** The scene for the proposed patch.*/
  QGraphicsScene* ProposedPatchScene;

  /** The item for the proposed patch.*/
  QGraphicsPixmapItem* ProposedPatchItem;

  /** The color to use as the background of the QGraphicsScenes */
  QColor SceneBackground;

  /** The model to use to display the patches. */
  ListModelPatches<TImage>* PatchesModel;

  /** These actions are performed when the widget is displayed. */
  void showEvent(QShowEvent* event);

  /** Gracefully quit if the user closes the dialog. */
  void closeEvent(QCloseEvent* event);

  /** Show the selected patch. */
  void DisplayPatchSelection();

  /** This variable tracks how many times the top patch was selected from the list. */
  unsigned int NumberOfVerifications = 0;

  /** This variable tracks how many times the patch was selected by manually positioning. */
  unsigned int NumberOfManualSelections = 0;

public:
  /** Constructor */
  PatchVerificationDialog(const TImage* const image, const Mask* const mask,
                          const unsigned int patchHalfWidth, QWidget* parent = nullptr);

  /** Output information about how many selections were used. */
  void Report();

  /** Set the source nodes from which the user can choose. */
  void SetSourceNode(const Node& sourceNode) override;

  /** Set the source nodes to display. */
  template <typename TNode>
  void SetSourceNode(const TNode& sourceNode);

  /** Set the query node that the user will choose the best match to. */
  void SetQueryNode(const Node& node) override;

  /** Update the source and target region outlines and display the source and target patches.*/
  void slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion,
                         const itk::ImageRegion<2>& targetRegion);

  /** The slot to handle when the btnAccept button is clicked. */
  void on_btnAccept_clicked() override;

  /** The slot to handle when the btnSelectManually button is clicked. */
  void on_btnSelectManually_clicked() override;

  /** Get the node that user selected. */
  Node GetSelectedNode() const;

  /** Position this dialog to the right of its parent. */
  void PositionNextToParent();
};

#include "PatchVerificationDialog.hpp"

#endif // PatchVerificationDialog_H
