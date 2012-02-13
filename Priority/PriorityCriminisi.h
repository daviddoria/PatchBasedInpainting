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

#include "PriorityOnionPeel.h"

/**
\class PriorityCriminisi
\brief This class implements Criminisi's priority function. It includes a Data term ontop of
       the confidence term of PriorityOnionPeel.
*/
template <typename TNode, typename TImage>
class PriorityCriminisi : public PriorityOnionPeel <TNode>
{
public:

  PriorityCriminisi(const TImage* const image, const Mask* const maskImage, unsigned int patchRadius);

  ///////////////////////////////////////////
  // Functions reimplemented from Priority //
  ///////////////////////////////////////////

  float ComputePriority(const TNode& queryPixel) const;

  void Update(const TNode& filledPixel);

//   std::vector<NamedVTKImage> GetNamedImages();
// 
//   static std::vector<std::string> GetImageNames();

  using PriorityOnionPeel<TNode>::ComputeConfidenceTerm;
  ///////////////////////////////////////////
  //////////////// New functions   //////////
  ///////////////////////////////////////////

  // Get the current data image
  FloatScalarImageType* GetDataImage();

protected:

  typedef PriorityOnionPeel<TNode> Superclass;

  // Compute the Data at a pixel.
  float ComputeDataTerm(const itk::Index<2>& queryPixel) const;

  // Compute the normals of the hole boundary.
  void ComputeBoundaryNormals(const float blurVariance);

  // Isophotes of the image.
  FloatVector2ImageType::Pointer IsophoteImage;

  // Boundary normals.
  FloatVector2ImageType::Pointer BoundaryNormalsImage;

private:
  const TImage* Image;
};

#include "PriorityCriminisi.hxx"

#endif
