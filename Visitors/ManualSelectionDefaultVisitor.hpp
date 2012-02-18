#ifndef ManualSelectionDefaultVisitor_HPP
#define ManualSelectionDefaultVisitor_HPP

// Custom
#include "Node.h"

// STL
#include <iostream>
#include <vector>

/**

 */
template <typename TVertexDescriptor>
class ManualSelectionDefaultVisitor
{
public:

  // ManualSelectionDefaultVisitor(){}

  /** Return the best source node for a specified target node. This default implementation
   *  simply returns the first node from the potenial set. */
  template <typename TForwardIterator>
  TVertexDescriptor select(const TVertexDescriptor& targetNode,
                           TForwardIterator possibleNodesBegin, TForwardIterator possibleNodesEnd)
  {
    return *possibleNodesBegin;
  }

}; // ManualSelectionDefaultVisitor

#endif
