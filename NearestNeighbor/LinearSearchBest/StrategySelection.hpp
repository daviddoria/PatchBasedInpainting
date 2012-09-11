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

#ifndef LinearSearchBestStrategySelection_HPP
#define LinearSearchBestStrategySelection_HPP

#include "HistogramDifference.hpp"

#include <Utilities/Histogram/HistogramHelpers.hpp>
#include <Utilities/Histogram/HistogramDifferences.hpp>

#include <Utilities/PatchHelpers.h>

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

  unsigned int Iteration;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestStrategySelection(PropertyMapType propertyMap, TImage* const image, Mask* const mask) :
    PropertyMap(propertyMap), Image(image), MaskImage(mask), Iteration(0)
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
    std::cout << "LinearSearchBestStrategySelection iteration: " << this->Iteration << std::endl;

    // If the input element range is empty, there is nothing to do.
    if(first == last)
    {
      std::cerr << "LinearSearchBestStrategySelection: Nothing to do..." << std::endl;
      return *last;
    }

    PatchHelpers::WriteTopPatches(this->Image, this->PropertyMap, first, last, "BestPatches", this->Iteration);

    typedef float BinValueType;
    typedef QuadrantHistogramProperties<typename TImage::PixelType> QuadrantHistogramPropertiesType;
    typedef MaskedHistogramGenerator<BinValueType, QuadrantHistogramPropertiesType> MaskedHistogramGeneratorType;
    typedef typename MaskedHistogramGeneratorType::QuadrantHistogramType QuadrantHistogramType;

    itk::ImageRegion<2> queryRegion = get(this->PropertyMap, query).GetRegion();

     // We must construct the bounds using the pixels from the entire valid region, otherwise the quadrant histogram
     // bins will not correspond to each other!
     bool useProvidedRanges = true;

     typename TImage::PixelType minValue = 0;

     typename TImage::PixelType maxValue = 255;

     // Use the range of the valid region of the target patch for the histograms. This is not necessarily the right thing
     // to do, because we might have a pretty solid "sky" patch that has more white in one of the quadrants than blue so
     // these quadrant histograms would match poorly even though the whole valid region was "sky" without any hard lines.
//     typename TImage::PixelType minValue;
//     MaskOperations::FindMinimumValueInMaskedRegion(this->Image, this->MaskImage,
//                                                    queryRegion, this->MaskImage->GetValidValue(), minValue);

//     typename TImage::PixelType maxValue;
//     MaskOperations::FindMaximumValueInMaskedRegion(this->Image, this->MaskImage,
//                                                    queryRegion, this->MaskImage->GetValidValue(), maxValue);

     QuadrantHistogramProperties<typename TImage::PixelType> quadrantHistogramProperties;

     quadrantHistogramProperties.SetAllMinRanges(minValue);
     quadrantHistogramProperties.SetAllMaxRanges(maxValue);

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
//         std::cout << "distance " << i << " " << j << " " << distance << std::endl;
         distances.push_back(distance);
       }
     }

     std::string logFileName = "StrategySelection.txt";
     std::ofstream fout(logFileName.c_str(), std::ios::app);

     // If there were no valid histograms to compare, just use the best SSD patch
     if(distances.size() == 0)
     {
       std::cout << "Using best SSD patch (not enough valid histogram quadrants)..." << std::endl;
       fout << "F " << Helpers::ZeroPad(this->Iteration, 3) << std::endl; // 'F'ail
       fout.close();
       this->Iteration++;
       return *first;
     }

     float maxDistance = Helpers::Max(distances);

     std::cout << "maxDistance: " << maxDistance << std::endl;
     bool useHistogramComparison = false;

     if(maxDistance < 0.5f)
     {
       useHistogramComparison = true;
     }

     if(useHistogramComparison)
     {
       std::cout << "Using histogram comparison with minValue: " << minValue
                 << " maxValue: " << maxValue << std::endl;
//       LinearSearchBestHistogramDifference<PropertyMapType, TImage, TIterator>
//           histogramCheck(this->PropertyMap, this->Image, this->MaskImage);
       LinearSearchBestDualHistogramDifference<PropertyMapType, TImage, TIterator>
           histogramCheck(this->PropertyMap, this->Image, this->MaskImage);
       histogramCheck.SetRangeMin(minValue);
       histogramCheck.SetRangeMax(maxValue);
       histogramCheck.SetNumberOfBinsPerDimension(30);

       fout << "H " << Helpers::ZeroPad(this->Iteration, 3)
            << " " << maxDistance << " : " << distances << std::endl; // 'H'istogram
       fout.close();
       this->Iteration++;
       return histogramCheck(first, last, query);
     }
     else // Just use the best SSD match
     {
       std::cout << "Using best SSD patch..." << std::endl;
       fout << "S " << Helpers::ZeroPad(this->Iteration, 3)
            << " " << maxDistance << " : " << distances << std::endl; // 'S'SD
       fout.close();
       this->Iteration++;
       return *first;
     }

  }

}; // end class

#endif
