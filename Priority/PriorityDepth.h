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

#include "Mask/Mask.h"

/**
\class PriorityDepth
\brief This class computes a patch's priority based on its depth channel.
*/
template <typename TNode, typename TImage>
class PriorityDepth
{
public:
  typedef itk::Image<float, 2> DepthImageType;
  
  typedef itk::CovariantVector<float, 2> FloatVector2Type;
  typedef itk::Image<FloatVector2Type , 2> FloatVector2ImageType;

  PriorityDepth(const TImage* const image, const Mask* const maskImage, const unsigned int patchRadius);

  // Implemented to model PriorityConcept
  float ComputePriority(const TNode& queryPixel) const;

  void Update(const TNode& filledPixel);

protected:
  // Isophotes of the depth channel.
  FloatVector2ImageType::Pointer DepthIsophoteImage;

  DepthImageType::Pointer DepthImage;

  const Mask* MaskImage;
  const TImage* Image;
};

#include "PriorityDepth.hpp"

#endif
