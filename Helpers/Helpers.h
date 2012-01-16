/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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

#ifndef HELPERS_H
#define HELPERS_H

// Custom
#include "Types.h"

namespace Helpers
{
//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Non-template function declarations (defined in Helpers.cpp) ///////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

bool IsValidRGB(const int r, const int g, const int b);

std::string ReplaceFileExtension(const std::string& fileName, const std::string& newExtension);

// Zero pad the 'iteration' and append it to the filePrefix, and add ".[fileExtension]" to the end.
// GetSequentialFileName("test", 2, "png");
// Produces "test_0002.png"
std::string GetSequentialFileName(const std::string& filePrefix, const unsigned int iteration, const std::string& fileExtension, const unsigned int paddedLength = 4);

// Patch sizes are specified by radius so they always have an odd side length. The side length is (2*radius)+1
unsigned int SideLengthFromRadius(const unsigned int radius);

// Convert a 'number' into a zero padded string.
// ZeroPad(5, 4); produces "0005"
std::string ZeroPad(const unsigned int number, const unsigned int rep);

// STL's .compare() function returns 0 when strings match, this is unintuitive.
bool StringsMatch(const std::string&, const std::string&);

// "Ceil()", but also for negative numbers.
// RoundAwayFromZero(.2) = 1
// RoundAwayFromZero(-.2) = -1
// (Normally ceil(-.2) = 0
float RoundAwayFromZero(const float number);

/////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Template function declarations (defined in Helpers.hxx) ///////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
unsigned int argmin(const T& vec);

template<typename T>
void NormalizeVector(T& v);

template<typename T>
typename T::value_type VectorMedian(T v);

// This pair of functions allows a scalar to be treated as the 0th component of a vector.
template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, T&>::type index(T& t, size_t);

template<typename T>
typename T::value_type& index(T& v, size_t i);

}// end namespace

#include "Helpers.hxx"

#endif