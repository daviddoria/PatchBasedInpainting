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

#include "Types.h"

class Mask;

namespace Derivatives
{
// Compute the derivative of an image in a specified 'direction' only using pixels that are Valid in the 'mask'.
// This is NOT the same as finding the unmasked derivative and then extracting only the valid pixels - the result
// can be very different depending on the 'image' values under the masked region.
template <typename TImage>
void MaskedDerivative(const TImage* image, const Mask* mask, const unsigned int direction, FloatScalarImageType* output);

template <typename TImage>
void MaskedDerivativePrewitt(const TImage* image, const Mask* mask, const unsigned int direction, FloatScalarImageType* output);

template <typename TImage>
void MaskedDerivativeSobel(const TImage* image, const Mask* mask, const unsigned int direction, FloatScalarImageType* output);

template <typename TImage>
void MaskedDerivativeGaussian(const TImage* image, const Mask* mask, const unsigned int direction, FloatScalarImageType* output);

template <typename TImage>
void MaskedDerivativeGaussianInRegion(const TImage* image, const Mask* mask, const unsigned int direction, const itk::ImageRegion<2>& region, FloatScalarImageType* output);

template <typename TImage>
void MaskedGradient(const TImage* image, const Mask* mask, FloatVector2ImageType* output);

template <typename TImage>
void MaskedGradientInRegion(const TImage* image, const Mask* mask, const itk::ImageRegion<2>& region, FloatVector2ImageType* output);

template<typename TPixel>
void GradientFromDerivatives(const itk::Image<TPixel, 2>* xDerivative, const typename itk::Image<TPixel, 2>* yDerivative, itk::Image<itk::CovariantVector<TPixel, 2> >* output);

template<typename TPixel>
void GradientFromDerivativesInRegion(const itk::Image<TPixel, 2>* xDerivative, const itk::Image<TPixel, 2>* yDerivative, const itk::ImageRegion<2>& region, itk::Image<itk::CovariantVector<TPixel, 2> >* output);

void ComputeMaskedIsophotesInRegion(const FloatScalarImageType* image, const Mask* mask, const itk::ImageRegion<2>& region, FloatVector2ImageType* outputIsophotes);

} // end namespace

#include "Derivatives.hxx"

#endif
