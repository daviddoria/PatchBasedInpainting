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

template <typename TImage>
void MaskedDerivative(const typename TImage::Pointer image, const Mask::Pointer mask, const unsigned int direction, FloatScalarImageType::Pointer output);

template <typename TImage>
void MaskedDerivativePrewitt(const typename TImage::Pointer image, const Mask::Pointer mask, const unsigned int direction, FloatScalarImageType::Pointer output);

template <typename TImage>
void MaskedDerivativeSobel(const typename TImage::Pointer image, const Mask::Pointer mask, const unsigned int direction, FloatScalarImageType::Pointer output);

template <typename TImage>
void MaskedDerivativeGaussian(const typename TImage::Pointer image, const Mask::Pointer mask, const unsigned int direction, FloatScalarImageType::Pointer output);

template <typename TImage>
void MaskedGradient(const typename TImage::Pointer image, const Mask::Pointer mask, FloatVector2ImageType::Pointer output);


template<typename TPixel>
void GradientFromDerivatives(const typename itk::Image<TPixel, 2>::Pointer xDerivative, const typename itk::Image<TPixel, 2>::Pointer yDerivative, typename itk::Image<itk::CovariantVector<TPixel, 2> >::Pointer output);

#include "Derivatives.hxx"

#endif
