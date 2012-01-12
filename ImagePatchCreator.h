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

#ifndef ImagePatchCreator_H
#define ImagePatchCreator_H

#include "ImagePatch.h"

/**
\class ItemCreator
\brief This is an abstract base class that produces an Item.
*/
template <typename TImage>
class ImagePatchCreator
{
public:

  ImagePatchCreator(const TImage* const image, const unsigned int patchRadius);

  Item* CreateItem(const itk::Index<2>& index) const;

private:
  const TImage* Image;
  unsigned int PatchRadius;
};

#include "ImagePatchCreator.hxx"

#endif
