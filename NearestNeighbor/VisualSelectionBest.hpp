#ifndef VisualSelectionBest_HPP
#define VisualSelectionBest_HPP

// Custom
#include "Node.h"
#include "Interactive/TopPatchesDialog.h"

// STL
#include <iostream>

/**
  Display a list of patches and let the user select the one to use.
 */
template <typename TImage>
class VisualSelectionBest : public QObject
{
private:
  TImage* Image;
  unsigned int PatchHalfWidth;
  TopPatchesDialog<TImage>* Dialog;

public:

  VisualSelectionBest(TImage* const image, const unsigned int patchHalfWidth, TopPatchesDialog<TImage>* dialog) :
  Image(image), PatchHalfWidth(patchHalfWidth), Dialog(dialog)
  {
  }

  /** Return the best source node for a specified target node. */
  template <typename TVertexDescriptor, typename TForwardIterator>
  TVertexDescriptor operator()(TForwardIterator possibleNodesBegin, TForwardIterator possibleNodesEnd,
                               const TVertexDescriptor& queryNode)
  {
//     TopPatchesWidget<TImage> topPatchesWidget(Image, PatchHalfWidth);
//     topPatchesWidget.show();

    // return *possibleNodesBegin; // As a placeholder, just return the first patch

    std::vector<Node> nodes;
    for(TForwardIterator iter = possibleNodesBegin; iter != possibleNodesEnd; ++iter)
      {
      Node node(*iter);
      nodes.push_back(node);
      }
    Dialog->SetNodes(nodes);

    QMetaObject::invokeMethod(Dialog, "exec", Qt::BlockingQueuedConnection);

    unsigned int selection = Dialog->GetSelectedItem();
    std::cout << "Selection: " << selection << std::endl;

    return *(possibleNodesBegin + selection);
  }

}; // VisualSelectionBest

#endif
