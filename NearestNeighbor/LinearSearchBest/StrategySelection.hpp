#ifndef LinearSearchBestStrategySelection_HPP
#define LinearSearchBestStrategySelection_HPP


#include "HistogramParent.hpp"

#include <Utilities/Histogram/HistogramHelpers.hpp>
#include <Utilities/Histogram/HistogramDifferences.hpp>

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
class LinearSearchBestStrategySelection
{
  PropertyMapType PropertyMap;
  TImage* Image;
  Mask* MaskImage;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestStrategySelection(PropertyMapType propertyMap, TImage* const image, Mask* const mask) :
    PropertyMap(propertyMap), Image(image), MaskImage(mask)
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
      std::cerr << "LinearSearchBestHistogram: Nothing to do..." << std::endl;
      return *last;
    }

    typedef int BinValueType;
    typedef QuadrantHistogramProperties<typename TImage::PixelType> QuadrantHistogramPropertiesType;
    typedef MaskedHistogramGenerator<BinValueType, QuadrantHistogramPropertiesType> MaskedHistogramGeneratorType;
    typedef typename MaskedHistogramGeneratorType::QuadrantHistogramType QuadrantHistogramType;

    itk::ImageRegion<2> queryRegion = get(this->PropertyMap, query).GetRegion();

     typename TImage::PixelType maxValue;
     MaskOperations::FindMaximumValueInMaskedRegion(this->Image, this->MaskImage, queryRegion, this->MaskImage->GetValidValue(), maxValue);

     typename TImage::PixelType minValue;
     MaskOperations::FindMinimumValueInMaskedRegion(this->Image, this->MaskImage, queryRegion, this->MaskImage->GetValidValue(), minValue);

     bool useProvidedRanges = false; // Compute the ranges internally
     QuadrantHistogramProperties<typename TImage::PixelType> quadrantHistogramProperties;

     QuadrantHistogramType targetQuadrantHistogram =
         MaskedHistogramGeneratorType::ComputeQuadrantMaskedImage1DHistogramAdaptive(this->Image, queryRegion,
           this->MaskImage, queryRegion, quadrantHistogramProperties,
           useProvidedRanges, this->MaskImage->GetValidValue());
     targetQuadrantHistogram.NormalizeHistograms();

     std::vector<float> distances;

     for(unsigned int i = 0; i < 4; ++i)
     {
       if(!targetQuadrantHistogram.Properties.Valid[i])
       {
         continue;
       }

       for(unsigned int j = 0; j < 4; ++j)
       {
         if(i == j)
         {
           continue;
         }

         if(!targetQuadrantHistogram.Properties.Valid[j])
         {
           continue;
         }

         typedef typename MaskedHistogramGeneratorType::HistogramType HistogramType;

         float distance = HistogramDifferences::HistogramDifference(targetQuadrantHistogram.Histograms[i], targetQuadrantHistogram.Histograms[j]);
         distances.push_back(distance);
       }
     }

     // If there were no valid histograms to compare, just use the best SSD patch
     if(distances.size() == 0)
     {
       return *first;
     }

     float maxDistance = Helpers::Max(distances);

     bool useHistogramComparison = false;

     if(maxDistance < 0.2f)
     {
       useHistogramComparison = true;
     }

     if(useHistogramComparison)
     {
       LinearSearchBestHistogramDifference<PropertyMapType, TImage, TIterator> linearSearchBestHistogramDifference(this->PropertyMap, this->Image, this->MaskImage);
       return linearSearchBestHistogramDifference(first, last, query);
     }
     else // Just use the best SSD match
     {
       return *first;
     }

  }

}; // end class

#endif
