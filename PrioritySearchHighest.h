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

#ifndef PrioritySearchHighest_H
#define PrioritySearchHighest_H

// Custom
#include "PixelCollection.h"
#include "Priority.h"
#include "Types.h"

// STL
#include <map>

/**
\class
\brief This class is responsible for computing the Priority at all pixels marked as boundary pixels.
       It is important to store the priority values that have been computed, because all but a few will
       remain unchanged from iteration to iteration.
*/
class PrioritySearchHighest
{
public:

  PrioritySearchHighest();

  itk::Index<2> FindHighestPriority(const PixelCollection& boundaryPixels, const Priority* const priority);

private:

  /** Priorities are tracked by storing the pixels and their priorities in a map. */
  typedef std::map<itk::Index<2>, float, itk::Index<2>::LexicographicCompare> PriorityMapType;

  /** Keep track of the priorities that have already been computed. */
  PriorityMapType PriorityMap;

  /** Compute the priorities of all specified pixels. */
  void ComputePriorities(const PixelCollection& boundaryPixels, const Priority* const priority);

  /** Compare the ->second elements of a map item (the map items values). */
  struct ValueCompareFunctor
  {
    bool operator()(PriorityMapType::value_type &i1, PriorityMapType::value_type &i2)
    {
      return i1.second < i2.second;
    }
  };

};

#include "PrioritySearchHighest.hxx"

#endif
