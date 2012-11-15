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

#ifndef PriorityConcept_HPP
#define PriorityConcept_HPP

#include <boost/concept_check.hpp>

#include "itkIndex.h"

/**
 * This concept-check class defines the functions that must be available for an object
 * to be interpreted as a Priority function.
 * 
 * \tparam TPriority The Priority function type.
 */
template <typename TPriority>
struct PriorityConcept 
{
  TPriority priority;

//  struct Node{};

  BOOST_CONCEPT_USAGE(PriorityConcept)
  {
//    Node node;
    itk::Index<2> node;

    // We must be able to compute the priority of a particular node
    priority.ComputePriority(node);

    // Many of the priority functions need to copy data from the source region to the target region,
    // so these functions must take both the source and target node location.
    priority.Update(node, node);
  }

};

#endif
