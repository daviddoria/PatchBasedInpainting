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

#ifndef PRIORITYDEPTH_H
#define PRIORITYDEPTH_H

#include "Priority.h"

/**
\class PriorityDepth
\brief This class computes a patch's priority based on its depth channel.
*/
template <typename TImage>
class PriorityDepth : public Priority
{
public:
  // Reimplemented from Priority
  PriorityDepth(const TImage* image, const Mask* maskImage, unsigned int patchRadius);

  float ComputePriority(const itk::Index<2>& queryPixel) const;

  void Update(const itk::Index<2>& filledPixel);

protected:
  // Isophotes of the depth channel.
  FloatVector2ImageType::Pointer DepthIsophoteImage;

  FloatScalarImageType::Pointer BlurredDepth;

  const Mask* MaskImage;
  const TImage* Image;
};

#include "PriorityDepth.hxx"

#endif
