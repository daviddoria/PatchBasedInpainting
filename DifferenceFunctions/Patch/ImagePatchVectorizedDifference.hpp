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

#ifndef ImagePatchVectorizedDifference_hpp
#define ImagePatchVectorizedDifference_hpp

// STL
#include <stdexcept>

// Custom
#include "PixelDescriptors/ImagePatchVectorized.h"

/** Compute the average difference between corresponding pixels in valid regions of the two patches.
 *  This is an average and not a sum because we want to be able to compare "match quality" values between
 *  different pairs of patches, in which the source region will not be the same size.
 */
template <typename ImagePatchType, typename DifferenceFunctorType>
struct ImagePatchVectorizedDifference
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

    float totalDifference = 0.0f;

    // If both nodes are source nodes, compare them fully.
    if(a.GetStatus() == ImagePatchType::SOURCE_NODE && b.GetStatus() == ImagePatchType::SOURCE_NODE)
    {
      typename std::vector<typename ImagePatchType::PixelType>::const_iterator iterA = a.GetPixelVector().begin();
      typename std::vector<typename ImagePatchType::PixelType>::const_iterator iterB = b.GetPixelVector().begin();
      for(;iterA != a.GetPixelVector().end(); ++iterA, ++iterB)
      {
        float pixelDifference = differenceFunctor(*iterA, *iterB);

        totalDifference += pixelDifference;
        }
      totalDifference = totalDifference / static_cast<float>(a.GetRegion().GetNumberOfPixels());
    }
    // If one of the nodes is a target node, only compare in it's list of valid offset pixels.
    else if(a.GetStatus() == ImagePatchType::TARGET_NODE || b.GetStatus() == ImagePatchType::TARGET_NODE)
    {
      typedef std::vector<unsigned int> OffsetVectorType;
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
        float difference = differenceFunctor(a.GetPixelVector()[*iter], b.GetPixelVector()[*iter]);
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
