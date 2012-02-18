#ifndef VisualSelectionBest_HPP
#define VisualSelectionBest_HPP

// Custom
#include "Node.h"

// Qt
#include <QObject>

// STL
#include <iostream>

/**

 */

// class VisualSelectionBestParent : public QObject
// {
// Q_OBJECT
// };

class VisualSelectionBest : public QObject
{
Q_OBJECT

signals:
  //void signal_Refresh();

  // void signal_Refresh();

public:

  // VisualSelectionBest(){}

  /** Return the best source node for a specified target node. */
  template <typename TVertexDescriptor, typename TForwardIterator>
  TVertexDescriptor operator()(TForwardIterator possibleNodesBegin, TForwardIterator possibleNodesEnd,
                               const TVertexDescriptor& queryNode)
  {
    // TODO: This should pop up a window and ask the user to select the patch they want to use (from the possibleNodes container)
  }

}; // VisualSelectionBest

#endif
