#ifndef ManualSelectionVisitor_HPP
#define ManualSelectionVisitor_HPP

// Custom
#include "Node.h"

// Qt
#include <QObject>

// STL
#include <iostream>
#include <vector>

/**

 */

// class ManualSelectionVisitorParent : public QObject
// {
// Q_OBJECT
// };

class ManualSelectionVisitor : public QObject
{
Q_OBJECT

signals:
  //void signal_Refresh();

  void signal_Refresh(const std::vector<Node>&);

public:

  // ManualSelectionVisitor(){}

  /** Return the best source node for a specified target node. */
  template <typename TForwardIterator>
  TVertexDescriptor select(const TVertexDescriptor& targetNode,
                           TForwardIterator possibleNodesBegin, TForwardIterator possibleNodesEnd)
  {
    // TODO: This should pop up a window and ask the user to select the patch they want to use (from the possibleNodes container)
  }

}; // ManualSelectionVisitor

#endif
