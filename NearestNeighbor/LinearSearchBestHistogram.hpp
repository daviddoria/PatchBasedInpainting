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

#ifndef LinearSearchBestHistogram_HPP
#define LinearSearchBestHistogram_HPP

// STL
#include <iostream>
#include <limits>

// Submodules
#include <Utilities/Histogram/MaskedHistogram.h>

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
template <typename PropertyMapType, typename TImage, typename TOriginalImage>
struct LinearSearchBestHistogram
{
private:
  /** The property map from which we will retrieve image regions from nodes. */
  PropertyMapType PropertyMap;

  /** The image to use in the histogram computations. */
  TImage* Image;

  /** The mask indicating which pixels (valid) to use in the histogram computations. */
  Mask* MaskImage;

  unsigned int NumberOfBinsPerDimension;

  // DEBUG ONLY
  unsigned int Iteration; // This is to keep track of which iteration we are on for naming debug output images
  /** This is the original (and intermediately inpainted image). We need this because
    * TIMage* Image is often not the RGB image (HSV or otherwise). */
  TOriginalImage* OriginalImage;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestHistogram(PropertyMapType propertyMap, TImage* const image, Mask* const mask, TOriginalImage* const originalImage) :
    PropertyMap(propertyMap), Image(image), MaskImage(mask), NumberOfBinsPerDimension(0), Iteration(0), OriginalImage(originalImage)
  {}

  void SetNumberOfBinsPerDimension(const unsigned int numberOfBinsPerDimension)
  {
    this->NumberOfBinsPerDimension = numberOfBinsPerDimension;
  }

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

    ITKHelpers::WriteSequentialImage(this->MaskImage, "HistogramMask", this->Iteration, 3, "png");
    ITKHelpers::WriteSequentialImage(this->OriginalImage, "OriginalImage", this->Iteration, 3, "png");
    ITKHelpers::WriteImage(this->Image, Helpers::GetSequentialFileName("Image", this->Iteration, "mha", 3));

    typedef int BinValueType;
    typedef Histogram<BinValueType>::HistogramType HistogramType;

    // This is the range of the HSV images we use.
    float rangeMin = 0.0f;
    float rangeMax = 1.0f;

    itk::ImageRegion<2> queryRegion = get(this->PropertyMap, query).GetRegion();

    {
    ITKHelpers::WriteRegionAsImage(this->OriginalImage, queryRegion, Helpers::GetSequentialFileName("QueryRegion",this->Iteration,"png",3));
    ITKHelpers::WriteRegionAsImage(this->OriginalImage, get(this->PropertyMap, *first).GetRegion(), Helpers::GetSequentialFileName("SSDRegion",this->Iteration,"png",3));
    }

    HistogramType queryHistogram =
      MaskedHistogram::ComputeMaskedImage1DHistogram(
                  this->Image, queryRegion, this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
                  rangeMin, rangeMax);

    typedef float DistanceValueType;

    typedef std::less<DistanceValueType> CompareFunctionType;
    CompareFunctionType compareFunction;

    // Initialize
    DistanceValueType d_best = std::numeric_limits<DistanceValueType>::infinity();
    TIterator result = last;

    // Iterate through all of the input elements
    for(TIterator current = first; current != last; ++current)
    {

      itk::ImageRegion<2> currentRegion = get(this->PropertyMap, *current).GetRegion();

      // Compute the histogram of the source region using the queryRegion mask
      HistogramType testHistogram =
          MaskedHistogram::ComputeMaskedImage1DHistogram(
                      this->Image, currentRegion, this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
                      rangeMin, rangeMax);

      float histogramDifference = Histogram<BinValueType>::HistogramDifference(queryHistogram, testHistogram);

      if(compareFunction(histogramDifference, d_best))
      {
        d_best = histogramDifference;
        result = current;
      }
    }

    std::cout << "Best histogramDifference: " << d_best << std::endl;
    ITKHelpers::WriteRegionAsImage(this->OriginalImage, get(this->PropertyMap, *first).GetRegion(), Helpers::GetSequentialFileName("HistogramRegion",this->Iteration,"png",3));

    this->Iteration++;

    return *result;
  }
};

#endif
