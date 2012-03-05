#ifndef WeightedImagePatchDifference_hpp
#define WeightedImagePatchDifference_hpp

// STL
#include <stdexcept>

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

template <typename ImagePatchType>
struct WeightedImagePatchDifference
{
  std::vector<float> Weights;

  float operator()(const ImagePatchType& a, const ImagePatchType& b) const
  {
    assert(Weights.size() == a.GetImage()->GetNumberOfComponentsPerPixel());
    
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
        float difference = 0.0f;
        for(unsigned int component = 0; component < a.GetImage()->GetNumberOfComponentsPerPixel(); ++component)
          {
          difference += Weights[component] * fabs(patchAIterator.Get()[component] - image->GetPixel(patchAIterator.GetIndex() + offsetAToB)[component]);
          }
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
        float difference = 0.0f;
        for(unsigned int component = 0; component < a.GetImage()->GetNumberOfComponentsPerPixel(); ++component)
          {
          difference += Weights[component] * fabs(image->GetPixel(a.GetCorner() + (*validOffsets)[i]) - image->GetPixel(b.GetCorner() + (*validOffsets)[i]));
          }
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

#endif
