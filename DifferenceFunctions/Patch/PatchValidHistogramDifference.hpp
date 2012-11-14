#ifndef PatchValidHistogramDifference_hpp
#define PatchValidHistogramDifference_hpp

// STL
#include <stdexcept>

/** Compute the average difference between corresponding pixels in valid regions of the two patches.
 *  This is an average and not a sum because we want to be able to compare "match quality" values between
 *  different pairs of patches, in which the source region will not be the same size.
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

  PatchValidHistogramDifference(const TImage* const image, const Mask* const mask) : Image(image), MaskImage(mask)
  {}

  float Difference(const RegionType& targetPatch, const RegionType& sourcePatch) const
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
                    this->Image, currentRegion, this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
                    this->RangeMin, this->RangeMax);

    // float histogramDifference = Histogram<BinValueType>::HistogramDifference(targetHistogram, testHistogram);
    float histogramDifference = Histogram<BinValueType>::WeightedHistogramDifference(targetHistogram, sourceHistogram);
//      float histogramDifference = Histogram<BinValueType>::HistogramCoherence(targetHistogram, testHistogram);

    return histogramDifference;
  }
};

#endif
