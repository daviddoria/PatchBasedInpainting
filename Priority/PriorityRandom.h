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

#ifndef PriorityRandom_H
#define PriorityRandom_H

/**
\class PriorityRandom
\brief This class returns a random value as the priority of each boundary pixel.
*/
template <typename TNode>
class PriorityRandom
{
public:
  typedef TNode NodeType;

  /** Return a random value.*/
  float ComputePriority(const TNode& queryPixel) const;

  /** There is no reason to update anything.*/
  void Update(const TNode& filledPixel){}

};

#include "PriorityRandom.hpp"

#endif
