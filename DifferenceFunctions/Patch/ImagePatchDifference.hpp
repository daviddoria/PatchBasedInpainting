/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef ImagePatchDifference_hpp
#define ImagePatchDifference_hpp

// STL
#include <stdexcept>

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

/** Compute the average difference between corresponding pixels in valid regions of the two patches.
 *  This is an average and not a sum because we want to be able to compare "match quality" values between
 *  different pairs of patches, in which the source region will not be the same size.
 *
 *  In this class, the 3 argument version of operator() does not assume that all input patches should be compared.
 */
template <typename ImagePatchType, typename PixelDifferenceFunctorType>
struct ImagePatchDifference
{
  PixelDifferenceFunctorType PixelDifferenceFunctor;

  ImagePatchDifference(PixelDifferenceFunctorType pixelDifferenceFunctor = PixelDifferenceFunctorType()) :
    PixelDifferenceFunctor(pixelDifferenceFunctor)
  {
//    pixelDifferenceFunctor.PrintName();
  }

  float operator()(const ImagePatchType& sourcePatch, const ImagePatchType& targetPatch) const
  {
    // If the source patch is invalid, the comparison cannot be performed.
//    if(sourcePatch.GetStatus() == ImagePatchType::INVALID)
    if(sourcePatch.GetStatus() != ImagePatchType::SOURCE_NODE)
    {
//      return std::numeric_limits<float>::infinity();
      return std::numeric_limits<float>::max();
    }

    // Require the source patch to be inside the image. This should be taken care of because the list of
    // source patches should all be inside the image
    assert(sourcePatch.IsInsideImage());
//    if(!sourcePatch.IsInsideImage())
//    {
//      return std::numeric_limits<float>::infinity();
//    }

    // If we are comparing a patch to itself, return inf. Otherwise, the best match would always be the same patch!
    // This should never be the case, because we do not compare target patches to target patches, and we do not compare
    // source patches to anything.
    assert(sourcePatch.GetRegion() != targetPatch.GetRegion());
//    if(sourcePatch.GetRegion() == targetPatch.GetRegion())
//    {
//      return std::numeric_limits<float>::infinity();
//    }

    typename ImagePatchType::ImageType* image = sourcePatch.GetImage();

    float totalDifference = 0.0f;

    typedef std::vector<itk::Offset<2> > OffsetVectorType;
    const OffsetVectorType* validOffsets = targetPatch.GetValidOffsetsAddress();

    assert(validOffsets->size() > 0);

    for(OffsetVectorType::const_iterator offsetIterator = validOffsets->begin();
        offsetIterator < validOffsets->end(); ++offsetIterator)
    {
      itk::Offset<2> currentOffset = *offsetIterator;
      itk::Index<2> currentTargetIndex = targetPatch.GetCorner() + currentOffset;
      typename ImagePatchType::ImageType::PixelType targetPixel =
          image->GetPixel(currentTargetIndex);

      typename ImagePatchType::ImageType::PixelType sourcePixel =
          image->GetPixel(sourcePatch.GetCorner() + currentOffset);

      float difference = this->PixelDifferenceFunctor(sourcePixel,
                                                      targetPixel);
      totalDifference += difference;
    }

    totalDifference = totalDifference / static_cast<float>(validOffsets->size());
    //std::cout << "Difference: " << totalDifference << std::endl;
    return totalDifference;
  }

  float operator()(const ImagePatchType& sourcePatch, const ImagePatchType& targetPatch,
                   const std::vector<typename ImagePatchType::ImageType::PixelType>& targetPixels) const
  {
    assert(targetPixels.size() == targetPatch.GetValidOffsetsAddress()->size());

    if(sourcePatch.GetStatus() != ImagePatchType::SOURCE_NODE)
    {
//      return std::numeric_limits<float>::infinity();
      return std::numeric_limits<float>::max();
    }

    typename ImagePatchType::ImageType* image = targetPatch.GetImage();

    float totalDifference = 0.0f;

    typedef std::vector<itk::Offset<2> > OffsetVectorType;
    const OffsetVectorType* validOffsets = targetPatch.GetValidOffsetsAddress();

    assert(validOffsets->size() > 0);

    for(OffsetVectorType::const_iterator offsetIterator = validOffsets->begin();
        offsetIterator < validOffsets->end(); ++offsetIterator)
    {
      itk::Offset<2> currentOffset = *offsetIterator;

      // Get the source pixel from the image
      itk::Index<2> currentSourceIndex = sourcePatch.GetCorner() + currentOffset;
      typename ImagePatchType::ImageType::PixelType sourcePixel =
          image->GetPixel(currentSourceIndex);

      // Get the target pixel from the pre-extracted contiguous memory
      typename ImagePatchType::ImageType::PixelType targetPixel =
          targetPixels[offsetIterator - validOffsets->begin()];

      float difference = this->PixelDifferenceFunctor(sourcePixel,
                                                      targetPixel);
      totalDifference += difference;
    }

    totalDifference = totalDifference / static_cast<float>(validOffsets->size());
    //std::cout << "Difference: " << totalDifference << std::endl;
    return totalDifference;
  }


};

#endif
