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

// Custom
#include "ImageProcessing/Mask.h"
#include "ImagePatchPixelDescriptor.h"

// STL
#include <set>

/**
\class SourcePatchCollection
\brief This class stores several fully valid Patch objects.
*/
template <typename TImage>
class SourcePatchCollection
{
public:

  SourcePatchCollection(const TImage* const image, const Mask* const maskImage, const unsigned int patchRadius);

  typedef std::set<ImagePatchPixelDescriptor<TImage> > PatchContainer;

  // Iterator interface
  typedef typename PatchContainer::iterator Iterator;
  //typedef std::set<Patch>::const_iterator const_iterator;
  Iterator begin() const;
  Iterator end() const;

  // Locate all patches centered at pixels in 'region' that are completely inside of the image and completely inside of the
  // source region and add them to the current list of source patches.
  // This function returns the set of newly added patches.
  void AddPatches(const PatchContainer& region);
  PatchContainer FindSourcePatchesInRegion(const itk::ImageRegion<2>& region);

  void Clear();

  const ImagePatchPixelDescriptor<TImage>* GetPatch(const itk::ImageRegion<2>& region);

  unsigned int GetNumberOfPatches() const;

private:

  PatchContainer SourcePatches;

  const TImage* Image;

  const Mask* MaskImage;

  unsigned int PatchRadius;

};

#include "SourcePatchCollection.hxx"

#endif
