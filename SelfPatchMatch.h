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

#ifndef SelfPatchMatch_H
#define SelfPatchMatch_H

#include "Helpers.h"
#include "Types.h"

#include "itkNeighborhoodAlgorithm.h"
#include "itkAndImageFilter.h"
#include "itkNumericTraits.h"
#include "itkImageRegion.h"

#include <iomanip> // setfill, setw
#include <vector>

template< typename TImage, typename TMask>
float PatchDifference(const TImage* image, const TMask* mask, itk::ImageRegion<2> sourceRegion, itk::ImageRegion<2> targetRegion);

template< typename TImage, typename TMask>
float PatchDifference(const TImage* image, const TMask* mask, itk::Index<2> queryPixel, itk::Index<2> currentPixel, unsigned int patchRadius, std::vector<float> weights);

template< typename TImage, typename TMask>
float PatchDifference(const TImage* image, const TMask* mask, itk::Index<2> queryPixel, itk::Index<2> currentPixel, unsigned int patchRadius);

template< typename TImage, typename TMask>
unsigned int BestPatch(const TImage* image, const TMask* mask, std::vector<itk::ImageRegion<2> > sourceRegions, itk::ImageRegion<2> targetRegion);

#include "SelfPatchMatch.txx"

#endif