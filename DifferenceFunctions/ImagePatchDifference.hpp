#ifndef ImagePatchDifference_hpp
#define ImagePatchDifference_hpp

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

    assert(a.GetImage() == b.GetImage());

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

    typename ImagePatchType::ImageType* image = a.GetImage(); // For now this image is required to be the same for both patches.

    float totalDifference = 0.0f;

    itk::Offset<2> offsetAToB = b.GetCorner() - a.GetCorner();

    // Compare all corresponding pixels and sum their differences
    itk::ImageRegionConstIteratorWithIndex<typename ImagePatchType::ImageType> imageIterator(image, a.GetRegion());
    while(!imageIterator.IsAtEnd())
      {
      //float difference = fabs(imageIterator.Get() - this->Image->GetPixel(imageIterator.GetIndex() + offsetToOther));
      float difference = (imageIterator.Get() - image->GetPixel(imageIterator.GetIndex() + offsetAToB)).GetNorm();
      totalDifference += difference;

      ++imageIterator;
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
