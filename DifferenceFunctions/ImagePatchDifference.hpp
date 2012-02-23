#ifndef ImagePatchDifference_hpp
#define ImagePatchDifference_hpp

// STL
#include <stdexcept>

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

/** Compute the average difference between corresponding pixels in valid regions of the two patches.
 *  This is an average and not a sum because we want to be able to compare "match quality" values between
 *  different pairs of patches, in which the source region will not be the same size.
 */
template <typename ImagePatchType, typename DifferenceFunctorType>
struct ImagePatchDifference
{
  float operator()(const ImagePatchType& a, const ImagePatchType& b) const
  {
    // This comparison must allow source patches to be compared to source patches (to create the tree) as well as source patches
    // to be symmetrically compared to target patches.

    DifferenceFunctorType differenceFunctor;

    if(!a.IsInsideImage() || !b.IsInsideImage())
      {
      return std::numeric_limits<float>::infinity();
      }

    // If we are comparing a patch to itself, return inf. Otherwise, the best match would always be the same patch!
    if(a.GetRegion() == b.GetRegion())
      {
      return std::numeric_limits<float>::infinity();
      }

    // If either patch is invalid, the comparison cannot be performed.
    if(a.GetStatus() == ImagePatchType::INVALID || b.GetStatus() == ImagePatchType::INVALID)
      {
      return std::numeric_limits<float>::infinity();
      }

    // For now this image is required to be the same for both patches.
    assert(a.GetImage() == b.GetImage());
    typename ImagePatchType::ImageType* image = a.GetImage();

    float totalDifference = 0.0f;

    // If both nodes are source nodes, compare them fully.
    if(a.GetStatus() == ImagePatchType::SOURCE_NODE && b.GetStatus() == ImagePatchType::SOURCE_NODE)
    {
      itk::Offset<2> offsetAToB = b.GetCorner() - a.GetCorner();
      // Compare all corresponding pixels and sum their differences
      itk::ImageRegionConstIteratorWithIndex<typename ImagePatchType::ImageType> patchAIterator(image, a.GetRegion());
      while(!patchAIterator.IsAtEnd())
        {
        float pixelDifference = differenceFunctor(patchAIterator.Get(), image->GetPixel(patchAIterator.GetIndex() + offsetAToB));

        totalDifference += pixelDifference;

        ++patchAIterator;
        }
      totalDifference = totalDifference / static_cast<float>(a.GetRegion().GetNumberOfPixels());
    }
    // If one of the nodes is a target node, only compare in it's list of valid offset pixels.
    else if(a.GetStatus() == ImagePatchType::TARGET_NODE || b.GetStatus() == ImagePatchType::TARGET_NODE)
    {
      typedef std::vector<itk::Offset<2> > OffsetVectorType;
      const OffsetVectorType* validOffsets;
      if(a.GetStatus() == ImagePatchType::TARGET_NODE)
        {
        validOffsets = a.GetValidOffsetsAddress();
        }
      if(b.GetStatus() == ImagePatchType::TARGET_NODE)
        {
        validOffsets = b.GetValidOffsetsAddress();
        }

      for(OffsetVectorType::const_iterator iter = validOffsets->begin(); iter < validOffsets->end(); ++iter)
        {
        float difference = differenceFunctor(image->GetPixel(a.GetCorner() + *iter), image->GetPixel(b.GetCorner() + *iter));
        totalDifference += difference;
        }
      totalDifference = totalDifference / static_cast<float>(validOffsets->size());
    }
    else
    {
      std::stringstream ss;
      ss << "Patch statuses are not correct! Status(a) is " << a.GetStatus() << " and Status(b) is " << b.GetStatus();
      throw std::runtime_error(ss.str());
    }

    //std::cout << "Difference: " << totalDifference << std::endl;
    return totalDifference;
  }
};

#endif
