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

#ifndef MaskOperations_H
#define MaskOperations_H

// Custom
#include "ImageTypes.h"
class Mask;

// ITK
#include "itkIndex.h"

namespace MaskOperations
{
// Look from a pixel across the hole in a specified direction and return the pixel that exists on the other side of the hole.
itk::Index<2> FindPixelAcrossHole(const itk::Index<2>& queryPixel, const FloatVector2Type& direction, const Mask* const mask);


// Apply the MaskedBlur function to every channel of a VectorImage separately.
void VectorMaskedBlur(const FloatVectorImageType* const inputImage, const Mask* const mask, const float blurVariance,
                      FloatVectorImageType* const output);


template <class TImage>
void CopySelfPatchIntoHoleOfTargetRegion(TImage* const image, const Mask* const mask,
                                         const itk::ImageRegion<2>& sourceRegionInput,
                                         const itk::ImageRegion<2>& destinationRegionInput);

template <class TImage>
void CopySourcePatchIntoHoleOfTargetRegion(const TImage* const sourceImage, TImage* const targetImage, const Mask* const mask,
                                           const itk::ImageRegion<2>& sourceRegionInput,
                                           const itk::ImageRegion<2>& destinationRegionInput);


template <typename TImage>
void MaskedBlur(const TImage* const inputImage, const Mask* const mask, const float blurVariance, TImage* const output);


template<typename TImage>
void CreatePatchImage(const TImage* const image, const itk::ImageRegion<2>& sourceRegion,
                      const itk::ImageRegion<2>& targetRegion, const Mask* const mask, TImage* const result);


// Return the highest value of the specified image out of the pixels under a specified BoundaryImage.
template<typename TImage>
itk::Index<2> FindHighestValueInMaskedRegion(const TImage* const image, float& maxValue, const Mask* const maskImage);

template<typename TImage, typename TRegionIndicatorImage>
itk::Index<2> FindHighestValueInNonZero(const TImage* const image, float& maxValue, const TRegionIndicatorImage* const maskImage);

/** Get the average value of the non-masked neighbors of a pixel. */
template<typename TImage>
typename TImage::PixelType AverageNonMaskedNeighborValue(const TImage* const image, const Mask* const mask,
                                                         const itk::Index<2>& pixel);

/** Get the average value of the masked neighbors of a pixel. */
template<typename TImage>
typename TImage::PixelType AverageMaskedNeighborValue(const TImage* const image, const Mask* const mask,
                                                      const itk::Index<2>& pixel);

} // end namespace

#include "MaskOperations.hxx"

#endif
