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

// This class includes the isophote direction term on top of the confidence term of PriorityOnionPeel.
class PriorityCriminisi : public PriorityOnionPeel
{
public:

  ///////////////////////////////////////////
  // Functions reimplemented from Priority //
  ///////////////////////////////////////////

  PriorityCriminisi(const FloatVectorImageType* image, const Mask* maskImage, unsigned int patchRadius);

  float ComputePriority(const itk::Index<2>& queryPixel);

  void Update(const itk::ImageRegion<2>& filledRegion);

  std::vector<NamedVTKImage> GetNamedImages();

  static std::vector<std::string> GetImageNames();

  ///////////////////////////////////////////
  //////////////// New functions   //////////
  ///////////////////////////////////////////

  // Get the current data image
  FloatScalarImageType* GetDataImage();

protected:

  // Compute the Data at a pixel.
  float ComputeDataTerm(const itk::Index<2>& queryPixel);

  // Compute the normals of the hole boundary.
  void ComputeBoundaryNormals(const float blurVariance);

  // Isophotes of the image.
  FloatVector2ImageType::Pointer IsophoteImage;

  // Boundary normals.
  FloatVector2ImageType::Pointer BoundaryNormalsImage;

};

#endif
