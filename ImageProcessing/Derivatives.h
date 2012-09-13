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

/**
 * Throughout this namespace, a TDifferenceFunction is specified. This is so that if we are computing the gradient
 * of say the H channel of an HSV image, we can specify the pixel-to-pixel difference function to use (we need a
 * wrapping distance function. Only the template parameter is necessary (as the difference function is only expected
 * to need an operator()(T, T), but we also accept an argument of an instance of the functor so that we can propagate
 * a functor through these functions (which sometimes call each other) using automatic deduction instead of having to
 * specify all of the other template parameters in order to specify this new one. That is, we would have to do:
 *
 * template <typename TImage, typename TGradientImage, typename TDifferenceFunction = std::minus<typename TImage::PixelType> >
 * void MaskedGradient(const TImage* const image, const Mask* const mask,
 *                      const itk::ImageRegion<2>& region, TGradientImage* const gradientImage)
 * {
 *    ...
 *    MaskedGradientInRegion<TImage, TGradientImage, TDifferenceFunction>(image, mask, image->GetLargestPossibleRegion(),
 *                                                                        gradientImage);
 * }
 *
 * instead of
 *
 * template <typename TImage, typename TGradientImage, typename TDifferenceFunction = std::minus<typename TImage::PixelType> >
 * void MaskedGradient(const TImage* const image, const Mask* const mask,
 *                      const itk::ImageRegion<2>& region, TGradientImage* const gradientImage,
 *                      TDifferenceFunction differenceFunction = TDifferenceFunction())
 * {
 *    ...
 *    MaskedGradientInRegion(image, mask, image->GetLargestPossibleRegion(), gradientImage, differenceFunction);
 * }
 *
 */
namespace Derivatives
{
// Compute the derivative of an image in a specified 'direction' only using pixels that are Valid in the 'mask'.
// This is NOT the same as finding the unmasked derivative and then extracting only the valid pixels - the result
// can be very different depending on the 'image' values under the masked region. E.g. if the pixels in the hole are
// black, the gradients will be very strong near the hole boundary (erroneously).
template <typename TImage, typename TDerivativeImage>
void MaskedDerivative(const TImage* const image, const Mask* const mask, const unsigned int direction, TDerivativeImage* const derivativeImage);

template <typename TImage, typename TDerivativeImage>
void MaskedDerivativePrewitt(const TImage* const image, const Mask* const mask, const unsigned int direction, TDerivativeImage* const derivativeImage);

template <typename TImage, typename TDerivativeImage>
void MaskedDerivativeSobel(const TImage* const image, const Mask* const mask, const unsigned int direction, TDerivativeImage* const derivativeImage);

template <typename TImage, typename TDerivativeImage, typename TDifferenceFunction = std::minus<typename TImage::PixelType> >
void MaskedDerivativeGaussian(const TImage* const image, const Mask* const mask, const unsigned int direction,
                              TDerivativeImage* const derivativeImage, TDifferenceFunction differenceFunction = TDifferenceFunction());

template <typename TImage, typename TDerivativeImage, typename TDifferenceFunction = std::minus<typename TImage::PixelType> >
void MaskedDerivativeGaussianInRegion(const TImage* const image, const Mask* const mask, const unsigned int direction,
                                      const itk::ImageRegion<2>& region, TDerivativeImage* const derivativeImage,
                                      TDifferenceFunction differenceFunction = TDifferenceFunction());

////////////// Gradients ////////////////

template <typename TImage, typename TGradientImage, typename TDifferenceFunction = std::minus<typename TImage::PixelType> >
void MaskedGradient(const TImage* const image, const Mask*const  mask, TGradientImage* const gradientImage,
                    TDifferenceFunction differenceFunction = TDifferenceFunction());

template <typename TImage, typename TGradientImage, typename TDifferenceFunction = std::minus<typename TImage::PixelType> >
void MaskedGradientInRegion(const TImage* const image, const Mask* const mask,
                            const itk::ImageRegion<2>& region, TGradientImage* const gradientImage,
                            TDifferenceFunction differenceFunction = TDifferenceFunction());

template<typename TPixel, typename TGradientImage>
void GradientFromDerivatives(const itk::Image<TPixel, 2>* const xDerivative,
                             const typename itk::Image<TPixel, 2>* const yDerivative, TGradientImage* const gradientImage);

template<typename TPixel, typename TGradientImage>
void GradientFromDerivativesInRegion(const itk::Image<TPixel, 2>* const xDerivative, const itk::Image<TPixel, 2>* const yDerivative,
                                     const itk::ImageRegion<2>& region, TGradientImage* const gradientImage);


} // end namespace

#include "Derivatives.hpp"

#endif
