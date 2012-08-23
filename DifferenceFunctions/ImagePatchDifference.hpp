#ifndef ImagePatchDifference_hpp
#define ImagePatchDifference_hpp

// STL
#include <stdexcept>

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

/** Compute the average difference between corresponding pixels in valid regions of the two patches.
 *  This is an average and not a sum because we want to be able to compare "match quality" values between
 *  different pairs of patches, in which the source region will not be the same size.
 */
template <typename ImagePatchType, typename PixelDifferenceFunctorType>
struct ImagePatchDifference
{
  PixelDifferenceFunctorType PixelDifferenceFunctor;

  ImagePatchDifference(PixelDifferenceFunctorType pixelDifferenceFunctor = PixelDifferenceFunctorType()) :
  PixelDifferenceFunctor(pixelDifferenceFunctor) {}

  float operator()(const ImagePatchType& sourcePatch, const ImagePatchType& targetPatch) const
  {
    // Require the source patch to be inside the image
    if(!sourcePatch.IsInsideImage())
    {
      return std::numeric_limits<float>::infinity();
    }

    // If we are comparing a patch to itself, return inf. Otherwise, the best match would always be the same patch!
    if(sourcePatch.GetRegion() == targetPatch.GetRegion())
    {
      return std::numeric_limits<float>::infinity();
    }

    // If the source patch is invalid, the comparison cannot be performed.
    if(sourcePatch.GetStatus() == ImagePatchType::INVALID)
    {
      return std::numeric_limits<float>::infinity();
    }

    typename ImagePatchType::ImageType* image = sourcePatch.GetImage();

    float totalDifference = 0.0f;

    typedef std::vector<itk::Offset<2> > OffsetVectorType;
    const OffsetVectorType* validOffsets = targetPatch.GetValidOffsetsAddress();

    for(OffsetVectorType::const_iterator iter = validOffsets->begin(); iter < validOffsets->end(); ++iter)
    {
      float difference = this->PixelDifferenceFunctor(image->GetPixel(sourcePatch.GetCorner() + *iter),
                                                      image->GetPixel(targetPatch.GetCorner() + *iter));
      totalDifference += difference;
    }
    totalDifference = totalDifference / static_cast<float>(validOffsets->size());
    //std::cout << "Difference: " << totalDifference << std::endl;
    return totalDifference;
  }
};

#endif
