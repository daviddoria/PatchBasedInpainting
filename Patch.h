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
#include "PixelVisitor.h"

class Mask;

/**
\class Patch
\brief This class indicates a rectangular region in an image.
       It is not associated with a particular image.
*/
class Patch
{
public:

  /** Construct a patch from a region.*/
  Patch(const itk::ImageRegion<2>& region);

  /** Check if two patches are the same.*/
  bool operator==(const Patch& other) const;

  /** Check if two patches are different.*/
  bool operator!=(const Patch& other) const;

  /** Get the region described by the patch.*/
  itk::ImageRegion<2> GetRegion() const;

  /** Get the corner of the patch.*/
  itk::Index<2> GetCorner() const;

  /** Sort the patches by index (so they can be stored in a container such as std::set).*/
  bool operator<(const Patch &other) const;

  /** Output information about the patch. */
  friend std::ostream& operator<<(std::ostream& output,  const Patch& patch);

  /** Visit all pixels in a patch.*/
  template <typename TImage>
  void VisitAllPixels(const TImage* const image, PixelVisitor<typename TImage::PixelType> &visitor);

  /** Visit all pixels in a patch where the mask value is valid.*/
  template <typename TImage>
  void VisitAllValidPixels(const TImage* const image, const Mask* const mask, PixelVisitor<typename TImage::PixelType> &visitor);

  /** Visit the pixels in a patch specified by a list of offsets from the corner of the patch. */
  template <typename TImage>
  void VisitOffsets(const TImage* const image, const std::vector<itk::Offset<2> >& offsets, PixelVisitor<typename TImage::PixelType> &visitor);

private:
  // The region in the image defining the patch.
  itk::ImageRegion<2> Region;

};

#include "Patch.hxx"

#endif
