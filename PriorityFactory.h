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

#ifndef PriorityFactory_H
#define PriorityFactory_H

#include <string>
#include <vector>

#include "Priority.h"

class PriorityFactory
{
public:
  static Priority* Create(const std::string& priorityType, FloatVectorImageType* const image, Mask* const maskImage, const unsigned int patchRadius);
  static std::vector<std::string> GetImageNames(const std::string& priorityType);
};

#endif
