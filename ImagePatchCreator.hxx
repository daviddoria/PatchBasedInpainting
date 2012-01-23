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

#include "ImagePatchCreator.h" // Appease syntax parser

#include "Helpers/ITKHelpers.h"

template <typename TImage>
ImagePatchCreator<TImage>::ImagePatchCreator(const TImage* const image, const unsigned int patchRadius) : Image(image), PatchRadius(patchRadius)
{

}

template <typename TImage>
Item* ImagePatchCreator<TImage>::CreateItem(const itk::Index<2>& index) const
{
  return new ImagePatchItem<TImage>(this->Image, ITKHelpers::GetRegionInRadiusAroundPixel(index, this->PatchRadius));
}
