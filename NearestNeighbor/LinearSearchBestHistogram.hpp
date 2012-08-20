#ifndef LinearSearchBestHistogram_HPP
#define LinearSearchBestHistogram_HPP

// STL
#include <iostream>
#include <limits>

// Submodules
#include <Utilities/Histogram/Histogram.h>

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
template <typename PropertyMapType, typename TImage>
struct LinearSearchBestHistogram
{
  PropertyMapType PropertyMap;

  TImage* Image;

  LinearSearchBestHistogram(PropertyMapType propertyMap, TImage* const image) :
    PropertyMap(propertyMap), Image(image)
  {}

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

    typedef int BinValueType;
    typedef Histogram<BinValueType>::HistogramType HistogramType;

    // This is the range of the HSV images we use.
    float rangeMin = 0.0f;
    float rangeMax = 1.0f;

    unsigned int numberOfBinsPerDimension = 30;

    HistogramType queryHistogram =
      Histogram<BinValueType>::ComputeImageHistogram1D(
                  this->Image, get(this->PropertyMap, query).GetRegion(), numberOfBinsPerDimension,
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
      HistogramType testHistogram =
          Histogram<BinValueType>::ComputeImageHistogram1D(
                      this->Image, get(this->PropertyMap, *current).GetRegion(), numberOfBinsPerDimension,
                      rangeMin, rangeMax);

      float histogramDifference = Histogram<BinValueType>::HistogramDifference(queryHistogram, testHistogram);

      if(compareFunction(histogramDifference, d_best))
      {
        d_best = histogramDifference;
        result = current;
      };
    };

    std::cout << "Best histogramDifference: " << d_best << std::endl;
    return *result;
  }
};

#endif
