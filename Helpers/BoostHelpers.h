#ifndef BoostHelpers_H
#define BoostHelpers_H

// STL
#include <iostream>

// Boost
#include <boost/array.hpp>

std::ostream& operator<<(std::ostream& output, const boost::array<size_t, 2>& vertexDescriptor);

#endif
