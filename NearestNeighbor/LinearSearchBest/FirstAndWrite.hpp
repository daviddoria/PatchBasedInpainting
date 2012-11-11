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

#ifndef LinearSearchBestFirstAndWrite_HPP
#define LinearSearchBestFirstAndWrite_HPP

#include <Utilities/PatchHelpers.h>

/** Write the N top patches (defined by the size of the grid passed to the
  * WriteTopPatchesGrid function)
  * and then return the best patch according to a distance function.
  * Note that this probably doesn't make sense if this is the only searcher used
  * because the top patches in the entire list are usually invalid (as they are
  * outside the image). Typically we would use this on the output of a KNNSearcher.
  */
template <typename TPatchDescriptorPropertyMap, typename TImage,
          typename TDistanceFunction>
class LinearSearchBestFirstAndWrite
{
  TPatchDescriptorPropertyMap PatchDescriptorPropertyMap;
  TImage* Image;
  Mask* MaskImage;
  TDistanceFunction DistanceFunction;

  unsigned int Iteration = 0;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestFirstAndWrite(TPatchDescriptorPropertyMap patchDescriptorPropertyMap,
                                TImage* const image, Mask* const mask,
                                TDistanceFunction distanceFunction = TDistanceFunction()) :
    PatchDescriptorPropertyMap(patchDescriptorPropertyMap), Image(image),
    MaskImage(mask), DistanceFunction(distanceFunction)
  {}

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  template <typename TIterator>
  typename TIterator::value_type operator()(const TIterator first, const TIterator last,
                                            typename TIterator::value_type query)
  {
    std::cout << "LinearSearchBestFirstAndWrite iteration: " << this->Iteration << std::endl;

    // If the input element range is empty, there is nothing to do.
    if(first == last)
    {
      std::cerr << "LinearSearchBestFirstAndWrite: Nothing to do..." << std::endl;
      return *last;
    }

//    PatchHelpers::WriteTopPatches(this->Image, this->PatchDescriptorPropertyMap, first, last,
//                                  "BestPatches", this->Iteration);

    unsigned int gridSize = 10;
    PatchHelpers::WriteTopPatchesGrid(this->Image, this->PatchDescriptorPropertyMap,
                                      first, last,
                                     "BestPatches", this->Iteration, gridSize, gridSize);

    LinearSearchBestProperty<TPatchDescriptorPropertyMap, TDistanceFunction>
        linearSearcher(this->PatchDescriptorPropertyMap);

    typename TIterator::value_type bestPatch = linearSearcher(first, last, query);

    this->Iteration++;

    return bestPatch;
  }

}; // end class

#endif
