#ifndef PrecomputedNeighbors_HPP
#define PrecomputedNeighbors_HPP

// STL
#include <sstream>
#include <stdexcept>

// Custom
#include "Node.h"
#include "Helpers/Helpers.h"

#include <boost/property_map/property_map.hpp>

struct PrecomputedNeighbors
{
  typedef std::map<Node, Node> NeighborMapType;
  NeighborMapType NeighborMap;
  
  PrecomputedNeighbors(std::ifstream& inputStream)
  {
    Node targetNode;
    Node sourceNode;
    std::string junk;
    std::string line;
    while(getline(inputStream, line))
      {
      std::stringstream ss;
      ss << line;
      ss >> targetNode[0] >> targetNode[1] >> junk >> sourceNode[0] >> sourceNode[1];
      NeighborMap[targetNode] = sourceNode;
      }
  }

  template <typename TForwardIterator>
  typename TForwardIterator::value_type operator()(TForwardIterator first, TForwardIterator last,
                                                   typename TForwardIterator::value_type query)
  {
    Node target = Helpers::ConvertFrom<Node, typename TForwardIterator::value_type>(query);
    NeighborMapType::iterator iter = NeighborMap.find(target);

    if(iter == NeighborMap.end())
      {
      std::stringstream ss;
      ss << "Target node " << target[0] << " " << target[1] << " was not found in the neighbor map!";
      throw std::runtime_error(ss.str());
      }
    else
      {
      typename TForwardIterator::value_type source = Helpers::ConvertFrom<typename TForwardIterator::value_type, Node>(iter->second);
      return source;
      }
  }
};

#endif
