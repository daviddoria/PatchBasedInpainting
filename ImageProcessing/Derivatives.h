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

#ifndef DERIVATVIES_H
#define DERIVATVIES_H

#include "ImageTypes.h"

class Mask;

namespace Derivatives
{
// Compute the derivative of an image in a specified 'direction' only using pixels that are Valid in the 'mask'.
// This is NOT the same as finding the unmasked derivative and then extracting only the valid pixels - the result
// can be very different depending on the 'image' values under the masked region.
template <typename TImage, typename TScalarImage>
void MaskedDerivative(const TImage* const image, const Mask* const mask, const unsigned int direction, TScalarImage* const output);

template <typename TImage, typename TScalarImage>
void MaskedDerivativePrewitt(const TImage* const image, const Mask* const mask, const unsigned int direction, TScalarImage* const output);

template <typename TImage, typename TScalarImage>
void MaskedDerivativeSobel(const TImage* const image, const Mask* const mask, const unsigned int direction, TScalarImage* const output);

template <typename TImage, typename TScalarImage>
void MaskedDerivativeGaussian(const TImage* const image, const Mask* const mask, const unsigned int direction, TScalarImage* const output);

template <typename TImage, typename TScalarImage>
void MaskedDerivativeGaussianInRegion(const TImage* const image, const Mask* const mask, const unsigned int direction,
                                      const itk::ImageRegion<2>& region, TScalarImage* const output);

template <typename TImage, typename TGradientImage>
void MaskedGradient(const TImage* const image, const Mask*const  mask, TGradientImage* const output);

template <typename TImage, typename TGradientImage>
void MaskedGradientInRegion(const TImage* const image, const Mask* const mask, const itk::ImageRegion<2>& region, TGradientImage* const output);

template<typename TPixel, typename TGradientImage>
void GradientFromDerivatives(const itk::Image<TPixel, 2>* const xDerivative,
                             const typename itk::Image<TPixel, 2>* const yDerivative, TGradientImage* const output);

template<typename TPixel, typename TGradientImage>
void GradientFromDerivativesInRegion(const itk::Image<TPixel, 2>* const xDerivative, const itk::Image<TPixel, 2>* const yDerivative,
                                     const itk::ImageRegion<2>& region, TGradientImage* const output);


} // end namespace

#include "Derivatives.hpp"

#endif
