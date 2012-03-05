#ifndef VisualSelectionBest_HPP
#define VisualSelectionBest_HPP

// Custom
#include "ImageProcessing/Mask.h"
#include "Node.h"
#include "Interactive/TopPatchesDialog.h"

// STL
#include <iostream>

// Qt
#include <QGraphicsScene>

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
  TopPatchesDialog<TImage>* Dialog;

public:

  VisualSelectionBest(TImage* const image, Mask* const mask, const unsigned int patchHalfWidth, TopPatchesDialog<TImage>* dialog) :
  Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth), Dialog(dialog)
  {

  }

  /** Return the best source node for a specified target node. */
  template <typename TVertexDescriptor, typename TForwardIterator>
  TVertexDescriptor operator()(TForwardIterator possibleNodesBegin, TForwardIterator possibleNodesEnd,
                               const TVertexDescriptor& queryVertex)
  {
    Node queryNode(queryVertex);
    //Dialog->SetQueryNode(queryNode); // Warnings that QPixmap is not safe to use in a non-GUI thread.
    // std::cout << "invokeMethod SetQueryNode." << std::endl;
    QMetaObject::invokeMethod(Dialog, "SetQueryNode", Qt::BlockingQueuedConnection, Q_ARG(Node, queryNode));

    std::vector<Node> sourceNodes;
    for(TForwardIterator iter = possibleNodesBegin; iter != possibleNodesEnd; ++iter)
      {
      Node node(*iter);
      // std::cout << "VisualSelectionBest::operator() node: " << node[0] << " " << node[1] << std::endl;
      sourceNodes.push_back(node);
      }
    //Dialog->SetSourceNodes(sourceNodes);
    // std::cout << "invokeMethod SetSourceNodes." << std::endl;
    QMetaObject::invokeMethod(Dialog, "SetSourceNodes", Qt::BlockingQueuedConnection, Q_ARG(std::vector<Node>, sourceNodes));

    // In Qt 4.7, we cannot get the return value of a function unless DirectConnection is used (which doesn't make sense in this
    // case since we need exec() to be called in the receiver's thread). Instead, we currently set the selected item to -1 so we
    // can tell if a valid selection has been made or not.
    QMetaObject::invokeMethod(Dialog, "exec", Qt::BlockingQueuedConnection);

    if(!Dialog->IsSelectionValid())
    {
      throw std::runtime_error("An invalid selection was made (IsSelectionValid returned false)!");
    }

    Node selectedNode = Dialog->GetSelectedNode();
    return Helpers::ConvertFrom<TVertexDescriptor, Node>(selectedNode);

  }

}; // VisualSelectionBest

#endif
