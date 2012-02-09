#ifndef ImagePatchDifference_hpp
#define ImagePatchDifference_hpp

// STL
#include <stdexcept>

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

/** Compute the difference to another ImagePatch.*/
// template <typename TImage>
// float Compare(const ImagePatchPixelDescriptor<TImage>* const a, const ImagePatchPixelDescriptor<TImage>* const b);

/** Compute the difference to another ImagePatch only at specified offets.*/
// float Compare(const ImagePatchPixelDescriptor* const item, const std::vector<itk::Offset<2> >& offsets) const;


template <typename ImagePatchType>
struct ImagePatchDifference
{
  float operator()(const ImagePatchType& a, const ImagePatchType& b) const
  {
    // This comparison must allow source patches to be compared to source patches (to create the tree) as well as source patches
    // to be symmetrically compared to target patches.

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

    if(a.GetStatus() == ImagePatchType::SOURCE_NODE && b.GetStatus() == ImagePatchType::SOURCE_NODE)
    {
      itk::Offset<2> offsetAToB = b.GetCorner() - a.GetCorner();
      // Compare all corresponding pixels and sum their differences
      itk::ImageRegionConstIteratorWithIndex<typename ImagePatchType::ImageType> patchAIterator(image, a.GetRegion());
      while(!patchAIterator.IsAtEnd())
        {
        //float difference = fabs(imageIterator.Get() - this->Image->GetPixel(imageIterator.GetIndex() + offsetToOther));
        float difference = (patchAIterator.Get() - image->GetPixel(patchAIterator.GetIndex() + offsetAToB)).GetNorm();
        totalDifference += difference;

        ++patchAIterator;
        }
    }
    else if(a.GetStatus() == ImagePatchType::TARGET_NODE || b.GetStatus() == ImagePatchType::TARGET_NODE)
    {
      const std::vector<itk::Offset<2> >* validOffsets;
      if(a.GetStatus() == ImagePatchType::TARGET_NODE)
        {
        validOffsets = a.GetValidOffsetsAddress();
        }
      if(b.GetStatus() == ImagePatchType::TARGET_NODE)
        {
        validOffsets = b.GetValidOffsetsAddress();
        }
      for(unsigned int i = 0; i < validOffsets->size(); ++i)
        {
        float difference = (image->GetPixel(a.GetCorner() + (*validOffsets)[i]) -
                            image->GetPixel(b.GetCorner() + (*validOffsets)[i])).GetNorm();
        totalDifference += difference;
        }
    }
    else
    {
      throw std::runtime_error("Patch statuses are not correct!");
    }

    //std::cout << "Difference: " << totalDifference << std::endl;
    return totalDifference;
  }
};


// template <typename TImage>
// float ImagePatchPixelDescriptor<TImage>::Compare(const ImagePatchPixelDescriptor* const other,
//                                                  const std::vector<itk::Offset<2> >& offsets) const
// {
//   if(!this->Valid || !other->IsValid())
//     {
//     return std::numeric_limits<float>::infinity();
//     }
//
//   float totalDifference = 0.0f;
//
//   for(unsigned int offsetId = 0; offsetId < offsets.size(); ++offsetId)
//   {
//     float difference = fabs(this->Image->GetPixel(this->GetCorner() + offsets[offsetId]) -
//                                this->Image->GetPixel(other->GetCorner() + offsets[offsetId]));
//     totalDifference += difference;
//   }
//
//   return totalDifference;
// }

#endif
