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

#include "Mask/Mask.h"

/**
\class PriorityOnionPeel
\brief This class ranks the priority of a patch based on its closeness to the hole boundary.
*/
class PriorityOnionPeel
{
public:

  PriorityOnionPeel(const Mask* const maskImage, const unsigned int patchRadius);

  ///////////////////////////////////////
  // Required to model PriorityConcept //
  ///////////////////////////////////////

  /** Compute the priority at a given pixel.*/
  template <typename TNode>
  float ComputePriority(const TNode& queryPixel) const;

  /** Update the priority function in the region around the target node.*/
  template <typename TNode>
  void Update(const TNode& sourceNode, const TNode& targetNode, const unsigned int patchNumber = 0);

protected:

  typedef itk::Image<float, 2> ConfidenceImageType;
  
  /** Compute the Confidence values for pixels that were just inpainted.*/
  template <typename TNode>
  void UpdateConfidences(const TNode& targetPixel, const float value);

  /** Compute the Confidence at a pixel.*/
  template <typename TNode>
  float ComputeConfidenceTerm(const TNode& queryPixel) const;

  /** Keep track of the Confidence of each pixel*/
  ConfidenceImageType::Pointer ConfidenceMapImage;

  /** The initial confidence is 0 in the hole and 1 outside the hole.*/
  void InitializeConfidenceMap();

  /** The mask image. */
  const Mask* MaskImage;

  /** The radius of the patch. */
  const unsigned int PatchRadius;
};

#include "PriorityOnionPeel.hxx"

#endif
