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

#ifndef GMHDifference_hpp
#define GMHDifference_hpp

// STL
#include <stdexcept>

// Custom
#include <ImageProcessing/Derivatives.h>

// Submodules
#include <Mask/Mask.h>
#include <Utilities/Histogram/HistogramGenerator.hpp>
#include <Utilities/Histogram/MaskedHistogramGenerator.hpp>
#include <Utilities/Histogram/HistogramDifferences.hpp>
#include <ITKHelpers/itkNormImageAdaptor.h>

// ITK
#include "itkNthElementImageAdaptor.h"

/** Compute the difference in Gradient Magnitude Histograms of two patches.
 */
template <typename TImage>
struct GMHDifference
{
  typedef itk::ImageRegion<2> RegionType;

  GMHDifference(const TImage* const image, const Mask* const mask,
                const unsigned int numberOfBinsPerChannel) :
    Image(image), MaskImage(mask), NumberOfBinsPerChannel(numberOfBinsPerChannel)
  {}

  float Difference(const RegionType& targetRegion, const RegionType& sourceRegion) const
  {
    // Define some types
    typedef float BinValueType; // bins must be float if we are going to normalize
    typedef MaskedHistogramGenerator<BinValueType> MaskedHistogramGeneratorType;
    typedef HistogramGenerator<BinValueType> HistogramGeneratorType;
    typedef HistogramGeneratorType::HistogramType HistogramType;

    // Setup storage of the gradients
    typedef itk::Image<itk::CovariantVector<float, 2>, 2> GradientImageType;

    // Extract the regions.
    typename TImage::Pointer targetRegionImage = TImage::New();
    ITKHelpers::ExtractRegion(this->Image, targetRegion, targetRegionImage.GetPointer());

    typename TImage::Pointer sourceRegionImage = TImage::New();
    ITKHelpers::ExtractRegion(this->Image, sourceRegion, sourceRegionImage.GetPointer());

    Mask::Pointer targetRegionMask = Mask::New();
    ITKHelpers::ExtractRegion(this->MaskImage, targetRegion, targetRegionMask.GetPointer());
    targetRegionMask->CopyInformationFrom(this->MaskImage);

    // The sourceRegion mask is always fully valid, but we extract it for symmetry in the following algorithm.
    Mask::Pointer sourceRegionMask = Mask::New();
    ITKHelpers::ExtractRegion(this->MaskImage, sourceRegion, sourceRegionMask.GetPointer());
    targetRegionMask->CopyInformationFrom(this->MaskImage);

    // Extract the channels and compute their derivatives
    typedef itk::NthElementImageAdaptor<TImage, float> ImageChannelAdaptorType;
    typename ImageChannelAdaptorType::Pointer targetImageChannelAdaptor = ImageChannelAdaptorType::New();
    targetImageChannelAdaptor->SetImage(targetRegionImage);

    typename ImageChannelAdaptorType::Pointer sourceImageChannelAdaptor = ImageChannelAdaptorType::New();
    sourceImageChannelAdaptor->SetImage(sourceRegionImage);

    // Initialize the final histograms. The channel histograms will be concatenated to form these final histograms.
    HistogramType targetHistogram;
    HistogramType sourceHistogram;

    // These are reused in the loop to store the gradients of a single channel at a time
    GradientImageType::Pointer sourceChannelGradients = GradientImageType::New();
    GradientImageType::Pointer targetChannelGradients = GradientImageType::New();

    // Compute the gradient of each channel
    for(unsigned int channel = 0; channel < 3; ++channel) // 3 is the number of RGB channels
    {
      sourceImageChannelAdaptor->SelectNthElement(channel);
      Derivatives::MaskedGradientInRegion(sourceImageChannelAdaptor.GetPointer(), sourceRegionMask,
                                          sourceRegionMask->GetLargestPossibleRegion(),
                                          sourceChannelGradients.GetPointer());

      targetImageChannelAdaptor->SelectNthElement(channel);
      Derivatives::MaskedGradientInRegion(targetImageChannelAdaptor.GetPointer(), targetRegionMask,
                                          targetRegionMask->GetLargestPossibleRegion(),
                                          targetChannelGradients.GetPointer());

      std::vector<itk::Index<2> > validPixels =
          ITKHelpers::GetPixelsWithValueInRegion(targetRegionMask.GetPointer(), targetRegionMask->GetLargestPossibleRegion(),
                                                 targetRegionMask->GetValidValue());

      typedef GradientImageType::PixelType::RealValueType ScalarType;
      // We use this filter to compute the magnitude of the gradient images
      typedef itk::NormImageAdaptor<GradientImageType, ScalarType>
          NormImageAdaptorType;
      typename NormImageAdaptorType::Pointer sourceNormImageAdaptor = NormImageAdaptorType::New();
      sourceNormImageAdaptor->SetImage(sourceChannelGradients.GetPointer());

      typename NormImageAdaptorType::Pointer targetNormImageAdaptor = NormImageAdaptorType::New();
      targetNormImageAdaptor->SetImage(targetChannelGradients.GetPointer());

      // Get the valid pixels
      std::vector<ScalarType> targetGradientMagnitudeValues =
          ITKHelpers::GetPixelValues(targetNormImageAdaptor.GetPointer(), validPixels);

      ScalarType minTargetGradientMagnitude = Helpers::Min(targetGradientMagnitudeValues);
      ScalarType maxTargetGradientMagnitude = Helpers::Max(targetGradientMagnitudeValues);

      // Compute histograms of the gradient magnitudes (to measure texture)
      bool allowOutside = false; // The histogram range should be fixed at the target range.
      HistogramType targetChannelHistogram =
        MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
            targetNormImageAdaptor.GetPointer(), targetNormImageAdaptor->GetLargestPossibleRegion(),
            targetRegionMask, targetRegionMask->GetLargestPossibleRegion(), this->NumberOfBinsPerChannel,
            minTargetGradientMagnitude, maxTargetGradientMagnitude,
            allowOutside, targetRegionMask->GetValidValue());

      targetChannelHistogram.Normalize();

      targetHistogram.Append(targetChannelHistogram);

      // Source region

      // The source region will not have the same range. We do not want to
      // throw an error if values are outside the range of the target
      // patch gradient magnitudes, but we want to include them by counting them on the extremal bins.
      allowOutside = true;

      HistogramType sourceChannelHistogram;

      // We don't need a masked histogram since we are using the full source patch.
      // Use the target histogram range.
      sourceChannelHistogram = HistogramGeneratorType::ComputeScalarImageHistogram(
            sourceNormImageAdaptor.GetPointer(), sourceNormImageAdaptor->GetLargestPossibleRegion(),
            this->NumberOfBinsPerChannel,
            minTargetGradientMagnitude,
            maxTargetGradientMagnitude, allowOutside);

      sourceChannelHistogram.Normalize();

      sourceHistogram.Append(sourceChannelHistogram);

    } // end loop over channels

    // Compute the differences in the histograms
    float histogramDifference = HistogramDifferences::HistogramDifference(targetHistogram, sourceHistogram);

    return histogramDifference;
  }

private:
  const TImage* Image;

  const Mask* MaskImage;

  unsigned int NumberOfBinsPerChannel;
};

#endif
