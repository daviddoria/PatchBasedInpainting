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

#ifndef IntroducedEnergy_H
#define IntroducedEnergy_H

#include <Mask/Mask.h>

/** When patches are copied into an image, there are two "energy" values that are worth studying.
  *
  * First, imagine if the entire source patch was copied into the target region. There is a potential for
  * a discontinuity around the boundary of the patch. For example, if we paste a solid red patch into a solid
  * green region, at the boundary of the patch there are red pixels adjacent to green pixels (a subset of
  * a square boundary shape). Though we dont actually copy these pixels in the valid region, we can use these
  * energies to determine if this is a reasonable patch to choose.
  *
  * Second, an actual energy that gets introduced is across the valid/hole boundary. If we copy a red patch
  * into a green region, this time though only in the hole region, we again have green pixels adjacent to red pixels
  * along the valid/hole boundary (arbitrarily shaped).
  */
template <typename TImage>
class IntroducedEnergy
{
  /** This function computes the energy introduced across the (valid) patch boundary by copying a patch into a region. */
  float ComputeIntroducedEnergyPatchBoundary(const TImage* const image, const Mask* const mask,
                                             const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

  /** This function computes the energy introduced across the (valid) patch boundary by copying a patch into a region. */
  float ComputeIntroducedEnergyMaskBoundary(const TImage* const image, const Mask* const mask,
                                            const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

  /** This function computes the total introduced energy by copying a patch into a region. */
  float ComputeIntroducedEnergy(const TImage* const image, const Mask* const mask,
                                const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

};

#include "IntroducedEnergy.hpp"

#endif
