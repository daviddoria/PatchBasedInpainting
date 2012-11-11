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

#ifndef LinearSearchBestProperty_HPP
#define LinearSearchBestProperty_HPP

// Submodules
#include <Utilities/Debug/Debug.h>

// STL
#include <iostream>
#include <limits>

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
template <typename PropertyMapType, typename PatchDistanceFunctionType>
struct LinearSearchBestProperty : public Debug
{
  PropertyMapType PropertyMap;
  PatchDistanceFunctionType PatchDistanceFunction;

  LinearSearchBestProperty(PropertyMapType propertyMap,
                           PatchDistanceFunctionType patchDistanceFunction = PatchDistanceFunctionType()) :
  PropertyMap(propertyMap), PatchDistanceFunction(patchDistanceFunction){}

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  template <typename TIterator>
  typename TIterator::value_type operator()(TIterator first, TIterator last, typename TIterator::value_type query)
  {
    // If the input element range is empty, there is nothing to do.
    if(first == last)
    {
      return *last;
    }

    // Initialize
    float d_best = std::numeric_limits<float>::infinity();

    typedef typename PropertyMapType::value_type PatchType;

    PatchType queryPatch = get(this->PropertyMap, query);

    typedef std::vector<typename PatchType::ImageType::PixelType> PixelVector;

    typedef std::vector<itk::Offset<2> > OffsetVectorType;
    const OffsetVectorType* validOffsets = queryPatch.GetValidOffsetsAddress();
    PixelVector targetPixels(validOffsets->size());

    for(OffsetVectorType::const_iterator offsetIterator = validOffsets->begin();
        offsetIterator < validOffsets->end(); ++offsetIterator)
    {
      itk::Offset<2> currentOffset = *offsetIterator;

      targetPixels[offsetIterator - validOffsets->begin()] =
          queryPatch.GetImage()->GetPixel(queryPatch.GetCorner() + currentOffset);
    }

    // Iterate through all of the input elements
    #pragma omp parallel for
//    for(TIterator current = first; current != last; ++current)
    for(TIterator current = first; current < last; ++current)
    {
      //DistanceValueType d = DistanceFunction(*first, query);
      float d = this->DistanceFunction(get(this->PropertyMap, *current), queryPatch, targetPixels);

      #pragma omp critical // There are weird crashes without this guard
      if(d < d_best)
      {
        d_best = d;
        result = current;
      }
    }

    std::cout << "Iteration " << this->DebugIteration << " search complete." << std::endl;

    this->DebugIteration++;

    return result;
  }
};

#endif
