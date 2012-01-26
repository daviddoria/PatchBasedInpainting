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
#include "itkImageRegion.h"

/**
\class Priority
\brief This is an abstract class that indicates the interface for Priority functions.
*/
class Priority
{
public:
  Priority();

  /** Compute the priority of a specific pixel. */
  virtual float ComputePriority(const itk::Index<2>& queryPixel) const = 0;

  virtual ~Priority(){}

  // At the end of an iteration, update anything that needs to be updated.
  virtual void Update(const itk::Index<2>& filledPixel) = 0;

};

#include "Priority.hxx"

#endif
