#ifndef Histogram_H
#define Histogram_H

namespace Histogram
{
  // If the MembershipImage is not provided, compute the histogram.
  std::vector<float> HistogramRegion(const FloatVectorImageType* image, const itk::ImageRegion<2>& imageRegion,
                                     const Mask* mask, const itk::ImageRegion<2>& maskRegion, const bool invertMask = false);
}

#endif
