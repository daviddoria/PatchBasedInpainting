#ifndef BoostHelpers_H
#define BoostHelpers_H

// STL
#include <iostream>

// Boost
#include <boost/array.hpp>

// Global namespace
std::ostream& operator<<(std::ostream& output, const boost::array<size_t, 2>& vertexDescriptor);

namespace BoostHelpers
{
  template <typename TQueue>
  void OutputQueue(TQueue queue);

}

#include "BoostHelpers.hpp"

#endif
