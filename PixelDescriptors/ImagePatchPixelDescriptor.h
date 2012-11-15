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

#include "PixelDescriptor.h"

// ITK
#include "itkImageRegion.h"

class Mask;

/**
\class ImagePatchPixelDescriptor
\brief This class indicates a rectangular region in an image.
*/
template <typename TImage>
class ImagePatchPixelDescriptor : public PixelDescriptor
{
public:

  typedef TImage ImageType;

  /** Default constructor to allow ImagePatch objects to be stored in a container.*/
  ImagePatchPixelDescriptor();

  /** Construct a patch from a region. Ideally 'image' would be const, but we also need a default constructor.*/
  ImagePatchPixelDescriptor(TImage* const image, Mask* const maskImage, const itk::ImageRegion<2>& region);

  /** Set the image which this region refers to.*/
  void SetImage(const TImage* const image);

  /** Set the region which this patch indicates.*/
  void SetRegion(const itk::ImageRegion<2>& region);

  /** Set the region which this patch oringially indicated before cropping.*/
  void SetOriginalRegion(const itk::ImageRegion<2>& region);

  /** Get the region described by the patch.*/
  itk::ImageRegion<2> GetRegion() const;

  /** Get the region described by the patch before it was potentially cropped.*/
  itk::ImageRegion<2> GetOriginalRegion() const;

  /** Get the corner of the patch.*/
  itk::Index<2> GetCorner() const;

  /** Output information about the patch. Even though this is inside a class template definition,
   * we still need to declare it as a function template. */
  template <typename T>
  friend std::ostream& operator<<(std::ostream& output,
                                  const ImagePatchPixelDescriptor<T>& patch);

  /** Get the image to which this patch refers. */
  TImage* GetImage() const;

  /** Determine if this patch is valid. A valid patch must be InsideImage, but also has had a valid descriptor attached to it. */
  bool IsFullyValid() const;

  void SetFullyValid(const bool fullyValid);

  void SetInsideImage(const bool insideImage);

  /** Determine if this patch is entirely within the image. */
  bool IsInsideImage() const;

  /** Set the valid offsets of the patch. */
  void SetValidOffsets(const std::vector<itk::Offset<2> >& validOffsets);

  /** Get the valid offsets of the patch. */
  std::vector<itk::Offset<2> > GetValidOffsets() const {return this->ValidOffsets;}

  /** Get the valid offsets of the patch. */
  const std::vector<itk::Offset<2> > * GetValidOffsetsAddress() const {return &this->ValidOffsets;}

private:
  /** The region in the image defining the location of the patch. */
  itk::ImageRegion<2> Region;

  /** The region in the image defining the location of the patch that is partially outside the image. */
  itk::ImageRegion<2> OriginalRegion;

  /** The image that the patch points to. */
  TImage* Image = nullptr;

  /** The Mask that describes the valid pixels in the patch. */
  Mask* MaskImage = nullptr;

  /** Indicate if every pixel in this image patch is valid or not. */
  bool FullyValid = false;

  /** Indicate if the patch region is entirely inside the image region. */
  bool InsideImage = false;

  /** A list of offsets from the patch corner that contain valid pixels.
      This is only used during the comparison to another patch if this patch has status TARGET_PATCH.*/
  std::vector<itk::Offset<2> > ValidOffsets;

};

#include "ImagePatchPixelDescriptor.hpp"

#endif
