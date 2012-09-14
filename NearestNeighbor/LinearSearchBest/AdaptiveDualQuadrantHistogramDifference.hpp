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

#ifndef LinearSearchBestAdaptiveDualQuadrantHistogramDifference_HPP
#define LinearSearchBestAdaptiveDualQuadrantHistogramDifference_HPP

#include "HistogramParent.hpp"

// Submodules
#include <Utilities/Histogram/Histogram.h>
#include <Utilities/Histogram/QuadrantHistogram.h>
#include <Utilities/Histogram/HistogramHelpers.hpp>

#include <Helpers/TypeTraits.h>

/**
   * This class computes both the "source hole pixels versus target valid pixels" histogram difference
   * as well as the "source valid pixels versus target valid pixels" histogram difference. These two scores
   * are combined by weighting by the number of valid pixels in the target patch. The range of the histograms
   * are determined by the source patch's target region value bounds.
   *
   * The code for traversing the container and finding the best item must be repeated in each of these classes
   * because we cannot simply use a distance functor that accepts (item, item) because we have to compute the
   * histogram of the target pixel once outside the loop, and then use it inside the loop (so that this
   * histogram does not need to be repeatedly computed at every iteration of the loop).
   *
   * This function template is similar to std::min_element but can be used when the comparison
   * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the
   * the element in the range [first,last) which has the "smallest" distance (of course, both the
   * distance metric and comparison can be overriden to perform something other than the canonical
   * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
   * \tparam PropertyMapType
   * \tparam TImage
   * \tparam TIterator
   * \tparam TImageToWrite
   */
template <typename PropertyMapType, typename TImage, typename TIterator, typename TImageToWrite = TImage>
class LinearSearchBestAdaptiveDualQuadrantHistogramDifference :
    public LinearSearchBestHistogramParent<PropertyMapType, TImage, TIterator, TImageToWrite>
{
public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestAdaptiveDualQuadrantHistogramDifference(PropertyMapType propertyMap, TImage* const image, Mask* const mask) :
    LinearSearchBestHistogramParent<PropertyMapType, TImage, TIterator, TImageToWrite>(propertyMap, image, mask)
  {}

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  typename TIterator::value_type operator()(const TIterator first, const TIterator last, typename TIterator::value_type query)
  {
    // If the input element range is empty, there is nothing to do.
    if(first == last)
    {
      std::cerr << "LinearSearchBestAdaptiveDualQuadrantHistogramDifference: Nothing to do..." << std::endl;
      return *last;
    }

//    typedef int BinValueType; // Can't use this if we are going to normalize the histograms.
    typedef float BinValueType;
    typedef QuadrantHistogramProperties<typename TImage::PixelType> QuadrantPropertiesType;
    typedef MaskedHistogramGenerator<BinValueType, QuadrantPropertiesType> MaskedHistogramGeneratorType;
    typedef typename MaskedHistogramGeneratorType::HistogramType HistogramType;
    typedef typename MaskedHistogramGeneratorType::QuadrantHistogramType QuadrantHistogramType;

    itk::ImageRegion<2> targetRegion = get(this->PropertyMap, query).GetRegion();

    QuadrantPropertiesType quadrantHistogramProperties;

    // Compute the quadrant histograms, to compare the valid regions of source/target patches
    bool useProvidedRanges = false; // We want to compute the ranges here, and then use them to compute the source patch histograms
    QuadrantHistogramType targetPatchValidRegionQuadrantHistogram =
        MaskedHistogramGeneratorType::ComputeQuadrantMaskedImage1DHistogramAdaptive
        (this->Image, targetRegion,
         this->MaskImage, targetRegion, quadrantHistogramProperties,
         useProvidedRanges,
         this->MaskImage->GetValidValue());

    targetPatchValidRegionQuadrantHistogram.NormalizeHistograms();

    // Find the valid pixels and determine the ranges to use to compute the non-quadrant histogram
    std::vector<itk::Index<2> > validPixelIndices = this->MaskImage->GetValidPixelsInRegion(targetRegion);
    std::vector<typename TImage::PixelType> validPixels = ITKHelpers::GetPixelValues(this->Image, validPixelIndices);
    unsigned int numberOfValidPixels = validPixels.size();

    typename TImage::PixelType rangeMins;
    Helpers::MinOfAllIndices(validPixels, rangeMins);

    typename TImage::PixelType rangeMaxs;
    Helpers::MaxOfAllIndices(validPixels, rangeMaxs);

    // Compute the non-quadrant histogram
    bool allowOutside = true;
    HistogramType targetPatchValidRegionHistogram =
        MaskedHistogramGeneratorType::ComputeMaskedImage1DHistogram(
          this->Image, targetRegion, this->MaskImage, targetRegion, this->NumberOfBinsPerDimension,
          rangeMins, rangeMaxs, allowOutside, this->MaskImage->GetValidValue());

    // As this will be compared to a histogram from a region with a different number of pixels, we must normalize it
    Helpers::NormalizeVectorInPlace(targetPatchValidRegionHistogram);

//    float percentValidPixels = this->MaskImage->CountValidPixels(queryRegion) / static_cast<float>(queryRegion.GetNumberOfPixels());

    if(this->WriteDebugPatches)
    {
      std::cout << "LinearSearchBestAdaptiveDualQuadrantHistogramDifference: Iteration "
                << this->Iteration << std::endl;
      ITKHelpers::WriteRegionAsRGBImage(this->ImageToWrite, targetRegion,
                                        Helpers::GetSequentialFileName("QueryRegion",this->Iteration,"png",3));
//      ITKHelpers::WriteScalarImageRegion(this->MaskImage, queryRegion, Helpers::GetSequentialFileName("QueryMask",this->Iteration,"png",3));
      ITKHelpers::WriteRegion(this->MaskImage, targetRegion,
                              Helpers::GetSequentialFileName("QueryMask",this->Iteration,"png",3));
      ITKHelpers::WriteImage(this->MaskImage, Helpers::GetSequentialFileName("Mask",this->Iteration,"png",3));
      ITKHelpers::WriteRegionAsRGBImage(this->ImageToWrite, get(this->PropertyMap, *first).GetRegion(),
                                        Helpers::GetSequentialFileName("SSDRegion",this->Iteration,"png",3));
      typename TImage::PixelType holeColor;
      holeColor[0] = 0;
      holeColor[1] = 255;
      holeColor[2] = 0;
      MaskOperations::WriteMaskedRegionPNG(this->ImageToWrite, this->MaskImage, targetRegion,
                                           Helpers::GetSequentialFileName("MaskedQueryRegion",this->Iteration,"png",3), holeColor);

      PatchHelpers::WriteTopPatches(this->ImageToWrite, this->PropertyMap, first, last,
                                    "TopPatches", this->Iteration);

      // Compute the histogram of the best SSD region using the queryRegion mask
//      HistogramType bestSSDHistogram =
//      // MaskedHistogramGeneratorType::ComputeMaskedImage1DHistogram(
//          MaskedHistogramGeneratorType::ComputeQuadrantMaskedImage1DHistogram(
//                      this->Image, get(this->PropertyMap, *first).GetRegion(), this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
//                      rangeMins, rangeMaxs);

//      float ssdMatchHistogramScore = Histogram<BinValueType>::HistogramDifference(targetHistogram, bestSSDHistogram);

//      std::cout << "Best SSDHistogramDifference: " << ssdMatchHistogramScore << std::endl;
//      HistogramHelpers::WriteHistogram(bestSSDHistogram, Helpers::GetSequentialFileName("BestSSDHistogram",this->Iteration,"txt",3));
//      HistogramHelpers::WriteHistogram(targetHistogram, Helpers::GetSequentialFileName("TargetHistogram",this->Iteration,"txt",3));
    }

    // Initialize
    float bestDistance = std::numeric_limits<float>::infinity();
    TIterator bestPatch = last;

    unsigned int bestId = 0; // Keep track of which of the top SSD patches is the best by histogram score (just for information sake)
    HistogramType bestHistogram;

    // Iterate through all of the input elements
    for(TIterator currentPatch = first; currentPatch != last; ++currentPatch)
    {
      itk::ImageRegion<2> currentRegion = get(this->PropertyMap, *currentPatch).GetRegion();

      // Compute the histogram of the source region using the target mask
      useProvidedRanges = true; // We want to compute these histograms using the ranges found for the target patch valid region
      QuadrantHistogramType sourcePatchValidRegionQuadrantHistogram =
          MaskedHistogramGeneratorType::ComputeQuadrantMaskedImage1DHistogramAdaptive
          (this->Image, currentRegion, this->MaskImage, targetRegion,
           targetPatchValidRegionQuadrantHistogram.Properties, useProvidedRanges,
           this->MaskImage->GetValidValue());

      // We must normalize this (even though it is being compared to a region with the same number of pixels)
      // so that the difference computed can be combined with the difference computed for the hole region (which DOES have to be normalized)
      sourcePatchValidRegionQuadrantHistogram.NormalizeHistograms();

      // Compute the difference between the quadrant histograms of the valid regions of the target/source patches
      auto quadrantDifferenceLambda = [](const HistogramType& a, const HistogramType& b)
        {
        return HistogramDifferences::HistogramDifference(a,b);
        };

      float validRegionsHistogramDifference = HistogramDifferences::QuadrantHistogramDifference(targetPatchValidRegionQuadrantHistogram, sourcePatchValidRegionQuadrantHistogram,
                                                                                                quadrantDifferenceLambda);

      // Compute the histogram of the source region hole pixels (using the target mask)
      HistogramType sourcePatchHoleRegionHistogram =
          MaskedHistogramGeneratorType::ComputeMaskedImage1DHistogram(
            this->Image, currentRegion, this->MaskImage, targetRegion, this->NumberOfBinsPerDimension,
            rangeMins, rangeMaxs, true, this->MaskImage->GetHoleValue());

      Helpers::NormalizeVectorInPlace(sourcePatchHoleRegionHistogram);

      // Compute the difference between the target patch valid region and the source patch hole region (to make sure we are not introducing any new colors)
      float holeValidHistogramDifference = HistogramDifferences::HistogramDifference(targetPatchValidRegionHistogram, sourcePatchHoleRegionHistogram);

      // Combine the two histogram differences by weighting them by how many pixels are in the valid region

      float histogramDifference = (numberOfValidPixels * validRegionsHistogramDifference) +
          (targetRegion.GetNumberOfPixels() - numberOfValidPixels) * holeValidHistogramDifference;

      if(histogramDifference < bestDistance)
      {
        bestDistance = histogramDifference;
        bestPatch = currentPatch;

        // These are not needed - just for debugging
        bestId = currentPatch - first;
        bestHistogram = sourcePatchHoleRegionHistogram;
      }
    }

    if(this->WriteDebugPatches)
    {
      itk::ImageRegion<2> bestRegion = get(this->PropertyMap, *bestPatch).GetRegion();
      ITKHelpers::WriteRegionAsRGBImage(this->ImageToWrite, bestRegion, Helpers::GetSequentialFileName("HistogramRegion",this->Iteration,"png",3));
      HistogramHelpers::WriteHistogram(bestHistogram, Helpers::GetSequentialFileName("BestHistogram",this->Iteration,"txt",3));
      std::cout << "Best histogram id: " << bestId << std::endl;
      std::cout << "Best histogramDifference: " << bestDistance << std::endl;
    }

    this->Iteration++;

//    if(this->Iteration == 50)
//    {
//      throw std::runtime_error("Quit at iteration 50 for profiling speed purposes.");
//    }

    return *bestPatch;
  }

}; // end class LinearSearchBestAdaptiveDualQuadrantHistogramDifference

#endif
