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

#ifndef PRIORITY_H
#define PRIORITY_H

#include "DebugOutputs.h"
class Mask;
#include "NamedVTKImage.h"
#include "Types.h"

class Priority : public DebugOutputs
{
public:
  Priority(const FloatVectorImageType* image, const Mask* maskImage, const unsigned int patchRadius);

  // Compute the priorities at all boundary pixels.
  virtual void ComputeAllPriorities();

  // At the end of an iteration, update anything that needs to be updated.
  virtual void Update(const itk::ImageRegion<2>& filledRegion);

  // Get the current priority image
  FloatScalarImageType::Pointer GetPriorityImage();

  // Get the current boundary image
  UnsignedCharScalarImageType::Pointer GetBoundaryImage();

  float GetPriority(const itk::Index<2>& queryPixel);

  void UpdateBoundary();

  virtual std::vector<NamedVTKImage> GetNamedImages();
  static std::vector<std::string> GetImageNames();

protected:

  // Compute the priority of a specific pixel.
  virtual float ComputePriority(const itk::Index<2>& queryPixel) = 0;

  // Keep track of the priority of each pixel.
  FloatScalarImageType::Pointer PriorityImage;

  // In most subclasses, the image and mask are needed to compute the priority.
  const FloatVectorImageType* Image;
  const Mask* MaskImage;

  // In most subclasses, the boundary image is needed to know where to compute the priority.
  UnsignedCharScalarImageType::Pointer BoundaryImage;

  unsigned int PatchRadius;
};

#endif
