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

struct Patch
{
public:

  Patch(const itk::ImageRegion<2>& region);

  bool operator==(const Patch& other) const;
  bool operator!=(const Patch& other) const;

  itk::ImageRegion<2> GetRegion() const;

  itk::Index<2> GetCorner() const;

  // Sort the patches by index (so they can be stored in a container such as std::set).
  bool operator<(const Patch &other) const;

  friend std::ostream& operator<<(std::ostream& output,  const Patch& patch);

  template <typename TImage>
  void VisitAllPixels(const TImage* const image, PixelVisitor<typename TImage::PixelType> &visitor);

  template <typename TImage>
  void VisitAllValidPixels(const TImage* const image, const Mask* const mask, PixelVisitor<typename TImage::PixelType> &visitor);

private:
  // The region in the image defining the patch.
  itk::ImageRegion<2> Region;

};

#include "Patch.hxx"

#endif
