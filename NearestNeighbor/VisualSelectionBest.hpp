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

#ifndef VisualSelectionBest_HPP
#define VisualSelectionBest_HPP

// Submodules
#include <Mask/Mask.h>

// Custom
#include "Node.h"
#include "Interactive/TopPatchesDialog.h"

// STL
#include <iostream>

// Qt
#include <QGraphicsScene>

class DialogHandlerParent : public QObject
{
Q_OBJECT

signals:
  void MySignal(Node* selectedNode);

public slots:
  virtual void MySlot(Node* selectedNode) = 0;

public:
  DialogHandlerParent()
  {
    connect( this, SIGNAL( MySignal(Node*) ), this, SLOT( MySlot(Node*) ), Qt::BlockingQueuedConnection);
  }

  void EmitSignal(Node* selectedNode)
  {
    std::cout << "DialogHandlerParent::EmitSignal" << std::endl;
    emit MySignal(selectedNode);
  }

};

template <typename TImage>
class DialogHandler : public DialogHandlerParent
{
  TImage* Image;
  Mask* MaskImage;
  unsigned int PatchHalfWidth;
  std::vector<Node> SourceNodes;
  Node QueryNode;

public:
  DialogHandler(TImage* image, Mask* mask, const unsigned int patchHalfWidth,
                std::vector<Node> sourceNodes, Node queryNode) :
    DialogHandlerParent(),
    Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth),
    SourceNodes(sourceNodes), QueryNode(queryNode)
  {
    std::cout << "DialogHandler::DialogHandler()" << std::endl;
  }

  void MySlot(Node* selectedNode)
  {
    std::cout << "DialogHandler::MySlot()" << std::endl;
    TopPatchesDialog<TImage>* topPatchesDialog =
        new TopPatchesDialog<TImage>(this->Image, this->MaskImage, this->PatchHalfWidth);
    topPatchesDialog->SetQueryNode(this->QueryNode);
    topPatchesDialog->SetSourceNodes(this->SourceNodes);
    topPatchesDialog->exec();

    if(!topPatchesDialog->IsSelectionValid())
    {
      throw std::runtime_error("An invalid selection was made (IsSelectionValid returned false)!");
    }

    *selectedNode = topPatchesDialog->GetSelectedNode();

    delete topPatchesDialog;
  }
};

/**
  Display a list of patches and let the user select the one to use.
 */
template <typename TImage>
class VisualSelectionBest : public QObject
{
private:
  TImage* Image;
  Mask* MaskImage;
  unsigned int PatchHalfWidth;

public:

  VisualSelectionBest(TImage* const image, Mask* const mask,
                      const unsigned int patchHalfWidth) :
  Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth)
  {

  }

  /** Return the best source node for a specified target node. */
  template <typename TVertexDescriptor, typename TForwardIterator>
  TVertexDescriptor operator()(TForwardIterator possibleNodesBegin, TForwardIterator possibleNodesEnd,
                               const TVertexDescriptor& queryVertex)
  {
//    std::cout << "VisualSelectionBest::operator()" << std::endl;
    std::cout << "There are " << possibleNodesEnd - possibleNodesBegin << " nodes." << std::endl;
    std::vector<Node> sourceNodes;
    for(TForwardIterator iter = possibleNodesBegin; iter != possibleNodesEnd; ++iter)
    {
      Node node(*iter);
      // std::cout << "VisualSelectionBest::operator() node: " << node[0] << " " << node[1] << std::endl;
      sourceNodes.push_back(node);
    }

    std::cout << "About to create dialogHandler" << std::endl;
    Node queryNode(queryVertex);
    DialogHandler<TImage>* dialogHandler = new DialogHandler<TImage>(this->Image, this->MaskImage, this->PatchHalfWidth,
                                                     sourceNodes, queryNode);
    dialogHandler->moveToThread(QCoreApplication::instance()->thread());

    Node selectedNode;
    dialogHandler->EmitSignal(&selectedNode);

    delete dialogHandler;

    return Helpers::ConvertFrom<TVertexDescriptor, Node>(selectedNode);
  }

}; // VisualSelectionBest

#endif
