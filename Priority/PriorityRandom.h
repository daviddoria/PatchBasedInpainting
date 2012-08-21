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

class PriorityRandom
{
public:
  /** Constructor that seeds the random number generator with the time.*/
  PriorityRandom();

  /** Constructor that seeds the random number generator with either the time or 0, depending on the value of 'random'.
    * If 'random' == false, we can reproduce results from one run to the next. */
  PriorityRandom(const bool random);

  /** Return a random value.*/
  template <typename TNode>
  float ComputePriority(const TNode& queryPixel) const;

  /** There is no reason to update anything.*/
  template <typename TNode>
  void Update(const TNode& sourceNode, const TNode& targetNode, const unsigned int patchNumber = 0){}

private:

};

#include "PriorityRandom.hpp"

#endif
