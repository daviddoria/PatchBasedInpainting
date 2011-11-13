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

#ifndef PRIORITYONIONPEEL_H
#define PRIORITYONIONPEEL_H

#include "Priority.h"
#include "Types.h"

class PriorityOnionPeel : public Priority
{
public:

  ///////////////////////////////////////////
  // Functions reimplemented from Priority //
  ///////////////////////////////////////////

  PriorityOnionPeel(FloatVectorImageType::Pointer image, Mask::Pointer maskImage, unsigned int patchRadius);

  float ComputePriority(const itk::Index<2>& queryPixel);

  void Update(const itk::ImageRegion<2>& filledRegion);

  ///////////////////////////////////////////
  //////////////// New functions   //////////
  ///////////////////////////////////////////

  // Get the current confidence map image
  FloatScalarImageType::Pointer GetConfidenceMapImage();

protected:

  // Compute the Confidence values for pixels that were just inpainted.
  void UpdateConfidences(const itk::ImageRegion<2>& targetRegion, const float value);

  // Compute the Confidence at a pixel.
  float ComputeConfidenceTerm(const itk::Index<2>& queryPixel);

  // Keep track of the Confidence of each pixel
  FloatScalarImageType::Pointer ConfidenceMapImage;

  // The initial confidence is 0 in the hole and 1 outside the hole.
  void InitializeConfidenceMap();

};

#endif
