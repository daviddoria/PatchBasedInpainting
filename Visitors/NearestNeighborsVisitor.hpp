#ifndef NearestNeighborsVisitor_HPP
#define NearestNeighborsVisitor_HPP

#include "Priority/Priority.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**
 
 */
struct NearestNeighborsVisitor
{
  template <typename TContainer>
  void FoundNeighbors(const TContainer& container)
  {
    std::cout << "NearestNeighborsVisitor: Found " << container.size() << " neighbors." << std::endl;
  }

}; // NearestNeighborsVisitor

#endif
