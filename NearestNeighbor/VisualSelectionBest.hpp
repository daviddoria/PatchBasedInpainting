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
#include "Interactive/DialogHandler.h"

// STL
#include <iostream>

// Qt
#include <QGraphicsScene>

/**
  Display a list of patches and let the user select the one to use.
  This class will be called from a non-GUI thread, so we cannot exec() the
  TopPatchesDialog directly. To rememdy this, we create a wrapper class that
  sends signals to the TopPatchesDialog that it puts in the GUI thread.
 */
template <typename TImage>
class VisualSelectionBest : public QObject
{
private:
  TImage* Image;
  Mask* MaskImage;
  unsigned int PatchHalfWidth;

  DialogHandler<TImage>* TopPatchesDialogHandler;

public:

  VisualSelectionBest(TImage* const image, Mask* const mask,
                      const unsigned int patchHalfWidth, QWidget* parent = nullptr) :
  Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth)
  {
    std::cout << "VisualSelectionBest parent pos " << parent->pos().x() << " " << parent->pos().y() << std::endl;
    this->TopPatchesDialogHandler =
        new DialogHandler<TImage>(this->Image, this->MaskImage, this->PatchHalfWidth, parent);
    this->TopPatchesDialogHandler->moveToThread(QCoreApplication::instance()->thread());
  }

  TopPatchesDialog<TImage>* GetTopPatchesDialog()
  {
    return this->TopPatchesDialogHandler->GetTopPatchesChooser();
  }

  ~VisualSelectionBest()
  {
    delete this->TopPatchesDialogHandler;
  }

  /** Return the best source node for a specified target node. */
  template <typename TVertexDescriptor, typename TForwardIterator>
  TVertexDescriptor operator()(TForwardIterator possibleNodesBegin, TForwardIterator possibleNodesEnd,
                               const TVertexDescriptor& queryVertex)
  {
//    std::cout << "VisualSelectionBest::operator()" << std::endl;
//    std::cout << "There are " << possibleNodesEnd - possibleNodesBegin << " nodes." << std::endl;

    std::vector<Node> sourceNodes;
    for(TForwardIterator iter = possibleNodesBegin; iter != possibleNodesEnd; ++iter)
    {
      Node node(*iter);
      // std::cout << "VisualSelectionBest::operator() node: " << node[0] << " " << node[1] << std::endl;
      sourceNodes.push_back(node);
    }

    this->TopPatchesDialogHandler->SetSourceNodes(sourceNodes);

    Node queryNode(queryVertex);
    this->TopPatchesDialogHandler->SetQueryNode(queryNode);

    // Signal the TopPatchesDialog and get the return value via this variable.
    Node selectedNode;
    this->TopPatchesDialogHandler->EmitSignal(&selectedNode);

    std::cout << "Returning selected node..." << std::endl;
    return Helpers::ConvertFrom<TVertexDescriptor, Node>(selectedNode);
  }

}; // VisualSelectionBest

#endif
