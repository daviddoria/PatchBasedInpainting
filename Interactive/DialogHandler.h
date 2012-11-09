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

#ifndef DialogHandler_H
#define DialogHandler_H

#include "TopPatchesDialog.h"

class DialogHandlerParent : public QObject
{
Q_OBJECT

signals:
  void signal_RunDialog(Node* selectedNode);

public slots:
  /** This function is implemented in DialogHandler. */
  virtual void slot_RunDialog(Node* selectedNode) = 0;

public:
  DialogHandlerParent()
  {
    /** When this class' signal is emitted, it should call its own slot. */
    connect(this, SIGNAL( signal_RunDialog(Node*) ), this, SLOT( slot_RunDialog(Node*) ),
            Qt::BlockingQueuedConnection);
  }

  /** This function should be called from a different thread.
    * It emits a signal that calls its own slot.
    */
  void EmitSignal(Node* selectedNode)
  {
    emit signal_RunDialog(selectedNode);
  }

};

template <typename TImage>
class DialogHandler : public DialogHandlerParent
{
  const TImage* Image;
  const Mask* MaskImage;
  unsigned int PatchHalfWidth;
  std::vector<Node> SourceNodes;
  Node QueryNode;

  TopPatchesDialog<TImage>* TopPatchesChooser;

public:
  DialogHandler(const TImage* image, const Mask* mask, const unsigned int patchHalfWidth,
                QWidget* parent = nullptr) :
    DialogHandlerParent(),
    Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth)
  {
    this->TopPatchesChooser =
        new TopPatchesDialog<TImage>(this->Image, this->MaskImage, this->PatchHalfWidth, parent);
  }

  TopPatchesDialog<TImage>* GetTopPatchesChooser()
  {
    return this->TopPatchesChooser;
  }

  ~DialogHandler()
  {
    delete this->TopPatchesChooser;
  }

  /** Give the handler the query node that it will pass to the GUI object. */
  void SetQueryNode(Node queryNode)
  {
    this->QueryNode = queryNode;
  }

  /** Give the handler the source nodes that it will pass to the GUI object. */
  void SetSourceNodes(std::vector<Node> sourceNodes)
  {
    this->SourceNodes = sourceNodes;
  }

  /** This function should only be called from a QueuedConnection. As long as DialogHandler
   * is in the GUI thread and it is called in this way, Qt is happy. */
  void slot_RunDialog(Node* selectedNode)
  {
    this->TopPatchesChooser->SetQueryNode(this->QueryNode);
    this->TopPatchesChooser->SetSourceNodes(this->SourceNodes);
    TopPatchesChooser->PositionNextToParent();
    this->TopPatchesChooser->exec();

    if(!this->TopPatchesChooser->IsSelectionValid())
    {
      throw std::runtime_error("An invalid selection was made (IsSelectionValid returned false)!");
    }

    *selectedNode = this->TopPatchesChooser->GetSelectedNode();

//    delete topPatchesDialog;
  }
};

#endif // DialogHandler_H
