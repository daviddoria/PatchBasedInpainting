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

#ifndef PRIORITYCRIMINISI_H
#define PRIORITYCRIMINISI_H

#include "Priority.h"
#include "Types.h"

class PriorityCriminisi : public Priority
{
public:

  ///////////////////////////////////////////
  // Functions reimplemented from Priority //
  ///////////////////////////////////////////

  PriorityCriminisi(FloatVectorImageType::Pointer image, Mask::Pointer maskImage, unsigned int patchRadius);

  float ComputePriority(const itk::Index<2>& queryPixel);

  void ComputeAllPriorities();
  ///////////////////////////////////////////
  //////////////// New functions   //////////
  ///////////////////////////////////////////

  // Get the current confidence image (confidences computed on the current boundary)
  FloatScalarImageType::Pointer GetConfidenceImage();

  // Get the current confidence map image
  FloatScalarImageType::Pointer GetConfidenceMapImage();

  // Get the current data image
  FloatScalarImageType::Pointer GetDataImage();

protected:

  // Compute the confidence values for pixels that were just inpainted.
  void UpdateConfidences(const itk::ImageRegion<2>& targetRegion, const float value);

  // Compute the Confidence at a pixel.
  float ComputeConfidenceTerm(const itk::Index<2>& queryPixel);

  // Compute the Data at a pixel.
  float ComputeDataTerm(const itk::Index<2>& queryPixel);

  // Keep track of the confidence of each pixel
  FloatScalarImageType::Pointer ConfidenceMapImage;

  // Store the computed confidences on the boundary
  FloatScalarImageType::Pointer ConfidenceImage;

  // Keep track of the data term of each pixel
  FloatScalarImageType::Pointer DataImage;

  // Isophotes of the image.
  FloatVector2ImageType::Pointer IsophoteImage;

  // Boundary normals.
  FloatVector2ImageType::Pointer BoundaryNormalsImage;

  // The initial confidence is 0 in the hole and 1 elsewhere.
  void InitializeConfidenceMap();

};

#endif
