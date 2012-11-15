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

#ifndef Priority_H
#define Priority_H

// ITK
#include "itkIndex.h"

// Submodules
#include <Utilities/Debug/Debug.h>

/**
\class Priority
\brief This is an abstract class to serve as a parent so that
       subclasses can be stored as parent pointers in a container.
       E.g. std::vector<Priority*> priorityFunctions
*/
class Priority : public Debug
{
public:

  // Would prefer this, but the language doesn't allow virtual function templates:
//   template <typename TNode>
//   virtual float ComputePriority(const TNode& queryPixel) = 0;
// 
//   template <typename TNode>
//   virtual void Update(const TNode& filledPixel) = 0;

  // Instead, we simply provide empty implementations
  template <typename TNode>
  float ComputePriority(const TNode& queryPixel)
  {
    throw std::runtime_error("Should not call Priority::ComputePriority()!");
  }

  template <typename TNode>
  void Update(const TNode& filledPixel, const TNode& targetNode,
              const unsigned int patchNumber = 0)
  {
    throw std::runtime_error("Should not call Priority::Update()!");
  }

  mutable unsigned int ComputePriorityCallCount = 0;

};

#endif
