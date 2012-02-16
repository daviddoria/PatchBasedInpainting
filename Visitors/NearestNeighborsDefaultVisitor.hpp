#ifndef NearestNeighborsDefaultVisitor_HPP
#define NearestNeighborsDefaultVisitor_HPP

#include "Priority/Priority.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**

 */
struct NearestNeighborsDefaultVisitor
{
  template <typename TContainer>
  void FoundNeighbors(const TContainer& container)
  {
    std::cout << "NearestNeighborsDefaultVisitor: Found " << container.size() << " neighbors." << std::endl;
  }

}; // NearestNeighborsDefaultVisitor

#endif
