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

/**
\class PriorityCriminisi
\brief This class implements Criminisi's priority function. It includes a Data term ontop of
       the confidence term of PriorityOnionPeel.
*/
template <typename TImage>
class PriorityCriminisi : public PriorityConfidence
{
public:

  PriorityCriminisi(const TImage* const image, const Mask* const maskImage, const unsigned int patchRadius);

  ///////////////////////////////////////////
  // Functions reimplemented from Priority //
  ///////////////////////////////////////////

  template <typename TNode>
  float ComputePriority(const TNode& queryPixel) const;

  template <typename TNode>
  void Update(const TNode& sourceNode, const TNode& targetNode, const unsigned int patchNumber = 0);

//   std::vector<NamedVTKImage> GetNamedImages();
// 
//   static std::vector<std::string> GetImageNames();

  using PriorityConfidence::ComputeConfidenceTerm;
  ///////////////////////////////////////////
  //////////////// New functions   //////////
  ///////////////////////////////////////////

  /** Get the current data image */
  void WriteDataImage(const std::string& fileName);

protected:

  typedef PriorityConfidence Superclass;

  /** Compute the Data at a pixel. */
  float ComputeDataTerm(const itk::Index<2>& queryPixel) const;

  /** Compute the normals of the hole boundary. */
  void ComputeBoundaryNormals(const float blurVariance);

  typedef itk::CovariantVector<float, 2> Vector2Type;
  typedef itk::Image<Vector2Type, 2> Vector2ImageType;

  /** Isophotes of the image. */
  Vector2ImageType::Pointer IsophoteImage;

  /** Boundary normals. */
  Vector2ImageType::Pointer BoundaryNormalsImage;

private:
  /** A pointer to the image we are inpainting */
  const TImage* Image;
};

#include "PriorityCriminisi.hpp"

#endif
