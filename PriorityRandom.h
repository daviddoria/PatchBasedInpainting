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

#ifndef PRIORITYRANDOM_H
#define PRIORITYRANDOM_H

#include "Priority.h"

// This class returns a random value as the priority of each boundary pixel.

template <typename TImage>
class PriorityRandom : public Priority<TImage>
{
public:
  PriorityRandom(const FloatVectorImageType* const image, const Mask* const maskImage, const unsigned int patchRadius);

  float ComputePriority(const itk::Index<2>& queryPixel);

  static std::vector<std::string> GetImageNames();
};

#include "PriorityRandom.hxx"

#endif
