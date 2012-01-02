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

#ifndef SourcePatchCollection_h
#define SourcePatchCollection_h

#include "Mask.h"
#include "Patch.h"

#include <set>

class SourcePatchCollection
{
public:
  SourcePatchCollection(Mask* const maskImage, const unsigned int patchRadius);

  // Iterator interface
  typedef std::set<Patch>::iterator iterator;
  //typedef std::set<Patch>::const_iterator const_iterator;
  iterator begin() const;
  iterator end() const;

  // Locate all patches centered at pixels in 'region' that are completely inside of the image and completely inside of the
  // source region and add them to the current list of source patches.
  // This function returns the set of newly added patches.
  std::set<Patch> AddSourcePatchesInRegion(const itk::ImageRegion<2>& region);

  void Clear();

private:

  std::set<Patch> SourcePatches;
  Mask* MaskImage;
  unsigned int PatchRadius;
};

#endif
