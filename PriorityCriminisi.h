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

#include "PriorityOnionPeel.h"
#include "Types.h"

class PriorityCriminisi : public PriorityOnionPeel
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

  // Get the current data image
  FloatScalarImageType::Pointer GetDataImage();

protected:

  // Compute the Data at a pixel.
  float ComputeDataTerm(const itk::Index<2>& queryPixel);

  // Compute the normals of the hole boundary.
  void ComputeBoundaryNormals(const float blurVariance);

  // Keep track of the data term of each pixel
  FloatScalarImageType::Pointer DataImage;

  // Isophotes of the image.
  FloatVector2ImageType::Pointer IsophoteImage;

  // Boundary normals.
  FloatVector2ImageType::Pointer BoundaryNormalsImage;

};

#endif
