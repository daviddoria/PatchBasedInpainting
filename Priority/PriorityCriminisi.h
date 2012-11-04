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

#ifndef PriorityCriminisi_H
#define PriorityCriminisi_H

#include "PriorityConfidence.h"

// Submodules
#include <Utilities/Debug/Debug.h>

/**
\class PriorityCriminisi
\brief This class implements Criminisi's priority function. It includes a Data term
       (based on the image gradient (isophotes) and the boundary normals) in addition
       to the confidence term (implemented in the parent class). It is recommended
       to blur the image ahead of time and provide the blurred image to this class,
       and ensure that that image is inpainted throughout the algorithm.
*/
template <typename TImage>
class PriorityCriminisi : public PriorityConfidence //, public Debug (this has to be done in the parent class
{
public:

//  PriorityCriminisi(const TImage* const image, const Mask* const maskImage,
//                    const unsigned int patchRadius);

  PriorityCriminisi(const typename TImage::Pointer image, const Mask* const maskImage,
                    const unsigned int patchRadius);
  /////////////////////////////////////////////////
  // Functions to satisfy the Priority interface //
  /////////////////////////////////////////////////

  template <typename TNode>
  float ComputePriority(const TNode& queryPixel) const;

  template <typename TNode>
  void Update(const TNode& sourceNode, const TNode& targetNode, const unsigned int patchNumber = 0);

  using PriorityConfidence::ComputeConfidenceTerm;

protected:

  typedef PriorityConfidence Superclass;

  /** Compute the Data at a pixel. */
  float ComputeDataTerm(const itk::Index<2>& queryPixel) const;

  typedef itk::CovariantVector<float, 2> Vector2Type;
  typedef itk::Image<Vector2Type, 2> Vector2ImageType;

  /** Isophotes of the image. */
  Vector2ImageType::Pointer IsophoteImage;

  /** Boundary normals. */
  Vector2ImageType::Pointer BoundaryNormalsImage;

  /** A pointer to the image we are inpainting */
//  const TImage* Image;
  const typename TImage::Pointer Image;

  /** Write the current data image. */
  void WriteDataImage(const unsigned int patchNumber);

  /** Write the current priority image. */
  void WritePriorityImage(const unsigned int patchNumber);

  /** Write the current boundary image. */
  void WriteBoundaryImage(const unsigned int patchNumber);

};

#include "PriorityCriminisi.hpp"

#endif
