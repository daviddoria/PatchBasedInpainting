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

#include "PatchBasedInpainting.h"

// Custom
#include "ClusterColors.h"
#include "HelpersOutput.h"

// Boost
#include <boost/bind.hpp>

FloatVectorImageType::Pointer PatchBasedInpainting::GetCurrentOutputImage()
{
  return this->CurrentInpaintedImage;
}

Mask* PatchBasedInpainting::GetMaskImage()
{
  return this->MaskImage;
}

void PatchBasedInpainting::SetPatchRadius(const unsigned int radius)
{
  // Since this is the radius of the patch, there are no restrictions for the radius to be odd or even.
  this->PatchRadius.Fill(radius);
}

unsigned int PatchBasedInpainting::GetPatchRadius()
{
  return this->PatchRadius[0];
}

itk::ImageRegion<2> PatchBasedInpainting::GetFullRegion()
{
  return this->FullImageRegion;
}

std::vector<CandidatePairs>& PatchBasedInpainting::GetPotentialCandidatePairsReference()
{
  // Return a reference to the whole set of forward look pairs.
  return PotentialCandidatePairs;
}

SelfPatchCompare* PatchBasedInpainting::GetPatchCompare() const
{
  return this->PatchCompare;
}

// void PatchBasedInpainting::SetPatchCompare(SelfPatchCompare* patchCompare)
// {
//   delete this->PatchCompare;
//   this->PatchCompare = patchCompare;
// }
