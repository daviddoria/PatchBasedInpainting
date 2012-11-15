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

#ifndef PatchValidHistogramDifference_hpp
#define PatchValidHistogramDifference_hpp

// STL
#include <stdexcept>

/** Compute the difference between the histogram of the valid region of the source patch and the
  * valid region of the target patch.
  *
  *  This should not be used if you are comparing the histogram of a target patch to the histograms of many source patches,
  *  as it will compute both histograms each time (where you could just compute the histogram of the target patch once and use
  *  it in each comparison).
  */
template <typename TImage>
struct PatchValidHistogramDifference
{
  typedef itk::ImageRegion<2> RegionType;

  const TImage* Image;

  const Mask* MaskImage;

  PatchValidHistogramDifference(const TImage* const image, const Mask* const mask) :
    Image(image), MaskImage(mask)
  {}

  float Difference(const RegionType& queryRegion, const RegionType& sourceRegion) const
  {
    typedef int BinValueType;

    HistogramType targetHistogram =
//      MaskedHistogram::ComputeMaskedImage1DHistogram(
        MaskedHistogram<BinValueType>::ComputeQuadrantMaskedImage1DHistogram(
                  this->Image, queryRegion, this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
                  this->RangeMin, this->RangeMax);

    HistogramType sourceHistogram =
//          MaskedHistogram::ComputeMaskedImage1DHistogram(
        MaskedHistogram<BinValueType>::ComputeQuadrantMaskedImage1DHistogram(
                    this->Image, sourceRegion, this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
                    this->RangeMin, this->RangeMax);

    // float histogramDifference = Histogram<BinValueType>::HistogramDifference(targetHistogram, testHistogram);
    float histogramDifference = Histogram<BinValueType>::WeightedHistogramDifference(targetHistogram, sourceHistogram);
//      float histogramDifference = Histogram<BinValueType>::HistogramCoherence(targetHistogram, testHistogram);

    return histogramDifference;
  }
};

#endif
