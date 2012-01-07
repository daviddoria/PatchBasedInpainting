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

#ifndef PRIORITYMANUAL_H
#define PRIORITYMANUAL_H

#include "PriorityOnionPeel.h"
#include "PriorityRandom.h"
#include "PriorityOnionPeel.h"
#include "PriorityDepth.h"

// This class first returns values that have been specified by a user generated image
// that contains pixels that indicate "definitely fill first". Once there are no more of
// these pixels, the superclass's ComputePriority function is called.

template <typename TPriority>
class PriorityManual : public TPriority
{
public:
  // Reimplemented
  PriorityManual(const FloatVectorImageType* image, const Mask* maskImage, unsigned int patchRadius);

  float ComputePriority(const itk::Index<2>& queryPixel);

  // New functions
  void SetManualPriorityImage(UnsignedCharScalarImageType* const);

  UnsignedCharScalarImageType* GetManualPriorityImage();

protected:
  UnsignedCharScalarImageType::Pointer ManualPriorityImage;
};

#include "PriorityManual.hxx"

#endif
