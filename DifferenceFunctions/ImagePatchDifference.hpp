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

    typedef std::vector<typename ImagePatchType::ImageType::PixelType> TargetPixelsContainerType;
    TargetPixelsContainerType targetPatchPixels(validOffsets->size());

    OffsetVectorType::const_iterator offsetIterator = validOffsets->begin();
    typename TargetPixelsContainerType::iterator targetPixelsIterator = targetPatchPixels.begin();
    for(; offsetIterator < validOffsets->end(); ++offsetIterator, ++targetPixelsIterator)
    {
      *targetPixelsIterator = image->GetPixel(targetPatch.GetCorner() + *offsetIterator);
    }

    offsetIterator = validOffsets->begin();
    targetPixelsIterator = targetPatchPixels.begin();
    for(; offsetIterator < validOffsets->end(); ++offsetIterator, ++targetPixelsIterator)
    {
      float difference = this->PixelDifferenceFunctor(image->GetPixel(sourcePatch.GetCorner() + *offsetIterator),
                                                     *targetPixelsIterator);
      totalDifference += difference;
    }
    totalDifference = totalDifference / static_cast<float>(validOffsets->size());
    //std::cout << "Difference: " << totalDifference << std::endl;
    return totalDifference;
  }
};

#endif
