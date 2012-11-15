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

#ifndef NodeConcept_HPP
#define NodeConcept_HPP

#include <boost/concept_check.hpp>

/**
 * This concept-check class defines the functions that must be available for an object to be interpreted as a node.
 *
 * \tparam TNode The node type.
 */
template <typename TNode>
struct NodeConcept 
{
  TNode node;

  BOOST_CONCEPT_USAGE(NodeConcept)
  {
    node[0];
  }

};

#endif
