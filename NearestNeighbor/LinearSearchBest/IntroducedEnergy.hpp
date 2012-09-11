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

#ifndef LinearSearchBestIntroducedEnergy_HPP
#define LinearSearchBestIntroducedEnergy_HPP

#include <Utilities/IntroducedEnergy.h>
#include <Utilities/Debug/Debug.h>

#include "LinearSearchBestParent.hpp"

/**
   * This function template is similar to std::min_element but can be used when the comparison
   * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the
   * the element in the range [first,last) which has the "smallest" distance (of course, both the
   * distance metric and comparison can be overriden to perform something other than the canonical
   * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
   * \tparam DistanceValueType The value-type for the distance measures.
   * \tparam DistanceFunctionType The functor type to compute the distance measure.
   * \tparam CompareFunctionType The functor type that can compare two distance measures (strict weak-ordering).
   */
template <typename TImagePatchDescriptorMap, typename TImage>
class LinearSearchBestIntroducedEnergy : public Debug, public LinearSearchBestParent
{
  TImage* Image;
  Mask* MaskImage;
  TImagePatchDescriptorMap ImagePatchDescriptorMap;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestIntroducedEnergy(TImagePatchDescriptorMap imagePatchDescriptorMap, TImage* const image, Mask* const mask) :
    Image(image), MaskImage(mask), ImagePatchDescriptorMap(imagePatchDescriptorMap)
  {}

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  template <typename TIterator>
  typename TIterator::value_type operator()(const TIterator first, const TIterator last, typename TIterator::value_type query)
  {
    // If the input element range is empty, there is nothing to do.
    if(first == last)
    {
      std::cerr << "LinearSearchBestIntroducedEnergy: Nothing to do..." << std::endl;
      return *last;
    }

    itk::ImageRegion<2> queryRegion = get(this->ImagePatchDescriptorMap, query).GetRegion();

    // Initialize
    float bestDistance = std::numeric_limits<float>::infinity();
    TIterator bestPatch = last;

    unsigned int bestId = 0; // Keep track of which of the top SSD patches is the best by histogram score (just for information sake)

    IntroducedEnergy<TImage> introducedEnergyFunctor;

    // Iterate through all of the input elements
    for(TIterator currentPatch = first; currentPatch != last; ++currentPatch)
    {
//      std::cout << "Iteration " << currentPatch - first << std::endl;

      itk::ImageRegion<2> currentRegion = get(this->ImagePatchDescriptorMap, *currentPatch).GetRegion();

      float introducedEnergy = introducedEnergyFunctor.ComputeIntroducedEnergy(this->Image, this->MaskImage,
                                                                                 currentRegion, queryRegion);

      if(introducedEnergy < bestDistance)
      {
        bestDistance = introducedEnergy;
        bestPatch = currentPatch;

        // These are not needed - just for debugging
        bestId = currentPatch - first;
      }
    }

    if(this->GetDebugOutputs())
    {
      std::cout << "Patch Id with best introduced energy: " << bestId << std::endl;
      std::cout << "Best introduced energy: " << bestDistance << std::endl;
    }

    if(this->GetDebugImages())
    {
      itk::ImageRegion<2> bestRegion = get(this->ImagePatchDescriptorMap, *bestPatch).GetRegion();
      ITKHelpers::WriteRegionAsRGBImage(this->Image, bestRegion, Helpers::GetSequentialFileName("BestIntroducedEnergyRegion",this->Iteration,"png",3));
    }

    this->Iteration++;

    return *bestPatch;
  }

}; // end class LinearSearchBestIntroduced

#endif
