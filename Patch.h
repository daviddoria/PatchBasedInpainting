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

#ifndef PATCH_H
#define PATCH_H

#include "Types.h"

struct Patch
{
public:

  Patch(const itk::ImageRegion<2>& region);

  bool operator==(const Patch& other) const;
  bool operator!=(const Patch& other) const;

  itk::ImageRegion<2> GetRegion() const;

  // Sort the patches by index (so they can be stored in a container such as std::set).
  bool operator<(const Patch &other) const;

private:
  // The region in the image defining the patch.
  itk::ImageRegion<2> Region;

  //float SortValue; // This simply allows patches to be sorted by any criterion currently being evaluated.
  //unsigned int Id; // This is used for parallel sorting of patches.
};

//bool SortBySortValue(const Patch& patch1, const Patch& patch2);

#endif
