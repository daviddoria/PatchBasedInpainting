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

#ifndef BoundaryEnergy_H
#define BoundaryEnergy_H

#include "Mask/Mask.h"

#include "itkVariableLengthVector.h"

/** This class computes the boundary image for the 'mask' in the specified
 * 'region' by computing the sum of the average source/target pixel difference
 * at each boundary pixel.
 */
template<typename TImage>
class BoundaryEnergy
{
public:
  BoundaryEnergy(const TImage* const image, const Mask* const mask);

  /** This functor assumes the source and target regions (the two sides of the boundary)
   * are adjacent (inside the same region in the image). */
  float operator()(const itk::ImageRegion<2>& region);

  /** This functor does not assume that the source and target regions (the two sides of the boundary)
   * are adjacent (inside the same region in the image). */
  float operator()(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

private:
  const TImage* Image;
  const Mask* MaskImage;

  /** Compute the difference between two pixels of unknown type. This will use operator-(). */
  template <typename T>
  float Difference(const T&, const T&);

  /** An overload to compute the difference between two vector pixels. */
  typedef itk::VariableLengthVector<float> VectorPixelType;
  float Difference(const VectorPixelType&, const VectorPixelType&);

};

#include "BoundaryEnergy.hpp"

#endif
