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

template <typename TPatchDescriptorPropertyMap, typename TImage, typename TDistanceFunction>
class LinearSearchBestFirstAndWrite
{
  TPatchDescriptorPropertyMap PatchDescriptorPropertyMap;
  TImage* Image;
  Mask* MaskImage;
  TDistanceFunction DistanceFunction;

  unsigned int Iteration;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestFirstAndWrite(TPatchDescriptorPropertyMap patchDescriptorPropertyMap,
                                TImage* const image, Mask* const mask,
                                TDistanceFunction distanceFunction = TDistanceFunction()) :
    PatchDescriptorPropertyMap(patchDescriptorPropertyMap), Image(image), MaskImage(mask), DistanceFunction(distanceFunction), Iteration(0)
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

    PatchHelpers::WriteTopPatches(this->Image, this->PatchDescriptorPropertyMap, first, last,
                                  "BestPatches", this->Iteration);

    std::ofstream fout(Helpers::GetSequentialFileName("Scores", this->Iteration, "txt", 3).c_str());

    for(TIterator current = first; current != last; ++current)
    {
      unsigned int patchId = current - first;
      typename TIterator::value_type sourceNode = *current;
      typename TIterator::value_type targetNode = query;

      typename TPatchDescriptorPropertyMap::value_type source = get(this->PatchDescriptorPropertyMap, sourceNode);
      typename TPatchDescriptorPropertyMap::value_type target = get(this->PatchDescriptorPropertyMap, targetNode);

//      typename TIterator::value_type source = get(this->PropertyMap, *current);
//      typename TIterator::value_type target = get(this->PropertyMap, query);
      float d = DistanceFunction(source, target); // (source, target) (the query node is the target node)

//      float d = DistanceFunction(get(this->PropertyMap, *current), get(this->PropertyMap, query)); // (source, target) (the query node is the target node)


      if(d < 0)
      {
        std::stringstream ss;
        ss << "LinearSearchBestFirstAndWrite: DistanceFunction returned a negative value!";
        throw std::runtime_error(ss.str());
      }
      fout << Helpers::ZeroPad(patchId, 3) << ": " << d << std::endl;
    }

    fout.close();

    this->Iteration++;

    return *first;
  }

}; // end class

#endif
