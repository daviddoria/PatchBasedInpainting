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

#include <vector>

class SourcePatchCollection
{
public:
  SourcePatchCollection(Mask* const maskImage, const unsigned int patchRadius);

  // Locate all patches centered at pixels in 'region' that are completely inside of the image and completely inside of the
  // source region and add them to the current list of source patches.
  void AddAllSourcePatchesInRegion(const itk::ImageRegion<2>& region);

  std::vector<Patch> AddNewSourcePatchesInRegion(const itk::ImageRegion<2>& region);

  std::vector<Patch*> GetSourcePatches();

  void Clear();

private:
  bool PatchExists(const itk::ImageRegion<2>& region);
  bool PatchExists(const Patch* const patch);
  std::vector<Patch*> SourcePatches; // TODO: why is this vector<pointer> instead of vector<object> ?
  Mask* MaskImage;
  unsigned int PatchRadius;
};

#endif
