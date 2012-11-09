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

#ifndef MatchVerificationVisitor_HPP
#define MatchVerificationVisitor_HPP

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

class MatchVerificationVisitor : public QObject
{
Q_OBJECT

signals:
  //void signal_Refresh();

  void signal_Refresh(const std::vector<Node>&);

public:

  // MatchVerificationVisitor(){}

  bool FoundNeighbors(const TContainer& container)
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

}; // MatchVerificationVisitor

#endif
