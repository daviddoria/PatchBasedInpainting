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

#ifndef Inpainting_h
#define Inpainting_h

#include "itkImage.h"

#include "Mask.h"

/**
\class Inpainting
\brief This is an abstract class to perform image inpainting.
*/
template <typename TImage>
class Inpainting
{
public:
  Inpainting(const TImage* const image, const Mask* mask) : Image(image), MaskImage(mask){}

  virtual void Inpaint() = 0;

private:
  TImage* Image;
  Mask* MaskImage;
  TImage* Result;
};

#endif
