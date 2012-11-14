#ifndef BoostHelpers_H
#define BoostHelpers_H

// STL
#include <iostream>
#include <string>

// Boost
#include <boost/array.hpp>

// Global namespace
std::ostream& operator<<(std::ostream& output, const boost::array<size_t, 2>& vertexDescriptor);

namespace BoostHelpers
{
  template <typename TQueue>
  void OutputQueue(TQueue queue);

  template <typename TPropertyMap, typename TImage>
  void WritePropertyMapAsImage(const TPropertyMap propertyMap, TImage* const image, const std::string& fileName);

  template <typename TNodeQueue, typename TPropertyMap, typename TImage>
  void WriteValidQueueNodesAsImage(TNodeQueue nodeQueue, const TPropertyMap propertyMap, TImage* const image, const std::string& fileName);

  template <typename TNodeQueue, typename TPropertyMap>
  unsigned int CountValidQueueNodes(TNodeQueue nodeQueue, const TPropertyMap propertyMap);

  template <typename TNodeQueue, typename TImage>
  void WriteAllQueueNodesAsImage(TNodeQueue nodeQueue, TImage* const image, const std::string& fileName);
}

#include "BoostHelpers.hpp"

#endif
