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

#ifndef Isophotes_H
#define Isophotes_H

#include "Mask/Mask.h"
#include "ImageTypes.h"

class Isophotes
{

public:
  /** This is a helper function that is called by ComputeColorIsophotesInRegion. */
template <typename TVectorImageType, typename TIsophoteImageType>
static void ComputeColorIsophotesInRegion(const TVectorImageType* image, const Mask* const mask,
                                          const itk::ImageRegion<2>& region , TIsophoteImageType* const isophotes);

private:
  /** This is a helper function that is called by ComputeColorIsophotesInRegion. */
  template <typename TScalarImageType, typename TIsophoteImageType>
  static void ComputeMaskedIsophotesInRegion(const TScalarImageType* const image, const Mask* const mask,
                                             const itk::ImageRegion<2>& region,
                                             TIsophoteImageType* const outputIsophotes);

};

#include "Isophotes.hpp"

#endif
