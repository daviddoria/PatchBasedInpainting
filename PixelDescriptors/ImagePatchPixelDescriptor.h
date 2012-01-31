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

#ifndef ImagePatchPixelDescriptor_H
#define ImagePatchPixelDescriptor_H

#include "PixelVisitor.h"
#include "Types.h"

class Mask;

/**
\class ImagePatchPixelDescriptor
\brief This class indicates a rectangular region in an image.
*/
template <typename TImage>
class ImagePatchPixelDescriptor
{
public:

  /** Default constructor to allow ImagePatch objects to be stored in a container.*/
  ImagePatchPixelDescriptor();

  /** Construct a patch from a region. Ideally 'image' would be const, but we also need a default constructor.*/
  ImagePatchPixelDescriptor(TImage* const image, Mask* const maskImage, const itk::ImageRegion<2>& region);

  /** Compute the difference to another ImagePatch.*/
  void SetImage(const TImage* const image);

  /** Compute the difference to another ImagePatch.*/
  void SetRegion(const itk::ImageRegion<2>& region);

  /** Check if two patches are the same.*/
  bool operator==(const ImagePatchPixelDescriptor& other) const;

  /** Check if two patches are different.*/
  bool operator!=(const ImagePatchPixelDescriptor& other) const;

  /** Get the region described by the patch.*/
  itk::ImageRegion<2> GetRegion() const;

  /** Get the corner of the patch.*/
  itk::Index<2> GetCorner() const;

  /** Output information about the patch. Even though this is inside a class template definition, we still need to declare it as a function template. */
  template <typename T>
  friend std::ostream& operator<<(std::ostream& output,  const ImagePatchPixelDescriptor<T>& patch);

  /** Get the image to which this patch refers. */
  TImage* GetImage() const;

  /** Determine if this patch is valid. A valid patch must be InsideImage, but also has had a valid descriptor attached to it. */
  bool IsFullyValid() const;

  /** Determine if this patch is entirely within the image. */
  bool IsInsideImage() const;

private:
  /** The region in the image defining the location of the patch. */
  itk::ImageRegion<2> Region;

  /** The image that the patch points to. */
  TImage* Image;

  /** The Mask that describes the valid pixels in the patch. */
  Mask* MaskImage;

  /** Indicate if every pixel in this image patch is valid or not. */
  bool FullyValid;

  /** Indicate if the patch region is entirely inside the image region. */
  bool InsideImage;

};


/** Compute the difference to another ImagePatch.*/
template <typename TImage>
float Compare(const ImagePatchPixelDescriptor<TImage>* const a, const ImagePatchPixelDescriptor<TImage>* const b);

  /** Compute the difference to another ImagePatch only at specified offets.*/
// float Compare(const ImagePatchPixelDescriptor* const item, const std::vector<itk::Offset<2> >& offsets) const;

#include "ImagePatchPixelDescriptor.hxx"

#endif
