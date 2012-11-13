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

#ifndef PatchVerificationDialogHandler_H
#define PatchVerificationDialogHandler_H

#include "PatchVerificationDialog.h"

class PatchVerificationDialogHandlerParent : public QObject
{
Q_OBJECT

signals:
  void signal_RunDialog(Node* selectedNode);

public slots:
  /** This function is implemented in DialogHandler. */
  virtual void slot_RunDialog(Node* selectedNode) = 0;

public:
  PatchVerificationDialogHandlerParent()
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
class PatchVerificationDialogHandler : public PatchVerificationDialogHandlerParent
{
  const TImage* Image;
  const Mask* MaskImage;
  unsigned int PatchHalfWidth;
  Node SourceNode;
  Node QueryNode;

  PatchVerificationDialog<TImage>* TopPatchVerifier = nullptr;

public:
  PatchVerificationDialogHandler(const TImage* image, const Mask* mask, const unsigned int patchHalfWidth,
                QWidget* parent = nullptr) :
    PatchVerificationDialogHandlerParent(),
    Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth)
  {
    this->TopPatchVerifier =
        new PatchVerificationDialog<TImage>(this->Image, this->MaskImage, this->PatchHalfWidth, parent);
  }

  PatchVerificationDialog<TImage>* GetTopPatchVerifier()
  {
    return this->TopPatchVerifier;
  }

  ~PatchVerificationDialogHandler()
  {
    delete this->TopPatchVerifier;
  }

  /** Give the handler the query node that it will pass to the GUI object. */
  void SetQueryNode(Node queryNode)
  {
    this->QueryNode = queryNode;
  }

  /** Give the handler the source nodes that it will pass to the GUI object. */
  void SetSourceNode(Node& sourceNode)
  {
    this->SourceNode = sourceNode;
  }

  /** This function should only be called from a QueuedConnection. As long as DialogHandler
   * is in the GUI thread and it is called in this way, Qt is happy. */
  void slot_RunDialog(Node* selectedNode) override
  {
    this->TopPatchVerifier->SetQueryNode(this->QueryNode);
    this->TopPatchVerifier->SetSourceNode(this->SourceNode);
    this->TopPatchVerifier->PositionNextToParent();
    this->TopPatchVerifier->exec();

    *selectedNode = this->TopPatchVerifier->GetSelectedNode();
  }
};

#endif // PatchVerificationDialogHandler_H
