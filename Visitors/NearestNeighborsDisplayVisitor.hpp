#ifndef NearestNeighborsDisplayVisitor_HPP
#define NearestNeighborsDisplayVisitor_HPP

// Custom
#include "Node.h"

// Qt
#include <QObject>

// STL
#include <iostream>
#include <vector>

/**
 
 */

// class NearestNeighborsDisplayVisitorParent : public QObject
// {
// Q_OBJECT
// };

class NearestNeighborsDisplayVisitor : public QObject
{
Q_OBJECT

signals:
  //void signal_Refresh();

  void signal_Refresh(const std::vector<Node>&);

public:

//   NearestNeighborsDisplayVisitor(){}

  template <typename TContainer>
  void FoundNeighbors(const TContainer& container)
  {
    std::cout << "NearestNeighborsDisplayVisitor: Found " << container.size() << " neighbors." << std::endl;
//     std::vector<Node> sortedNodes;
//     for(TContainer::iterator iter = container.begin(); iter != container.end(); ++iter)
//       {
//       Node node((*iter)[0], (*iter)[1]);
//       sortedNodes.push_back(node);
//       }
//     std::sort(sortedNodes.begin(), sortedNodes.end());

    std::vector<Node> nodes;
    for(typename TContainer::const_iterator iter = container.begin(); iter != container.end(); ++iter)
      {
      Node node((*iter)[0], (*iter)[1]);
      nodes.push_back(node);
      }

    emit signal_Refresh(nodes);
  }

}; // NearestNeighborsDisplayVisitor

#endif
