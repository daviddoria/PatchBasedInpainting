#ifndef ImagePatchPixelDescriptor_H
#define ImagePatchPixelDescriptor_H

#include "PixelDescriptor.h"

/**
\class ImagePatchPixelDescriptor
\brief Describes a pixel by it's surrounding region in an image.
*/
template <typename TImage>
class ImagePatchPixelDescriptor : public PixelDescriptor
{
  /** The image from which the pixel comes from. */
  TImage* Image;

  /** The region surrounding the pixel. */
  itk::ImageRegion<2> Region;
};

#endif
