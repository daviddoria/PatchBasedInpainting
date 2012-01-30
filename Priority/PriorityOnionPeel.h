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

/**
\class PriorityOnionPeel
\brief This class ranks the priority of a patch based on its closeness to the hole boundary.
*/
class PriorityOnionPeel : public Priority
{
public:

  ///////////////////////////////////////////
  // Functions reimplemented from Priority //
  ///////////////////////////////////////////

  PriorityOnionPeel(const Mask* const maskImage, const unsigned int patchRadius);

  virtual ~PriorityOnionPeel(){}

  float ComputePriority(const itk::Index<2>& queryPixel) const;

  void Update(const itk::Index<2>& filledPixel);

protected:

  /** Compute the Confidence values for pixels that were just inpainted.*/
  void UpdateConfidences(const itk::Index<2>& targetPixel, const float value);

  /** Compute the Confidence at a pixel.*/
  float ComputeConfidenceTerm(const itk::Index<2>& queryPixel) const;

  /** Keep track of the Confidence of each pixel*/
  FloatScalarImageType::Pointer ConfidenceMapImage;

  /** The initial confidence is 0 in the hole and 1 outside the hole.*/
  void InitializeConfidenceMap();

  /** The mask image. */
  const Mask* MaskImage;

  /** The radius of the patch. */
  const unsigned int PatchRadius;
};

#include "PriorityOnionPeel.hxx"

#endif
