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
    std::cout << "invokeMethod SetQueryNode." << std::endl;
    QMetaObject::invokeMethod(Dialog, "SetQueryNode", Qt::BlockingQueuedConnection, Q_ARG(Node, queryNode));

    std::vector<Node> sourceNodes;
    for(TForwardIterator iter = possibleNodesBegin; iter != possibleNodesEnd; ++iter)
      {
      Node node(*iter);
      sourceNodes.push_back(node);
      }
    //Dialog->SetSourceNodes(sourceNodes);
    std::cout << "invokeMethod SetSourceNodes." << std::endl;
    QMetaObject::invokeMethod(Dialog, "SetSourceNodes", Qt::BlockingQueuedConnection, Q_ARG(std::vector<Node>, sourceNodes));

    QMetaObject::invokeMethod(Dialog, "exec", Qt::BlockingQueuedConnection);

    unsigned int selection = Dialog->GetSelectedItem();
    std::cout << "Selection: " << selection << std::endl;

    return *(possibleNodesBegin + selection);
  }

}; // VisualSelectionBest

#endif
