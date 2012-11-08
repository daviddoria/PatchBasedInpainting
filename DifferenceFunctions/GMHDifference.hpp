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

  /** 'image' can't be const because NthElementImageAdaptor won't allow it. */
  GMHDifference(TImage* const image, Mask* const mask,
                const unsigned int numberOfBinsPerChannel) :
    Image(image), MaskImage(mask), NumberOfBinsPerChannel(numberOfBinsPerChannel)
  {}

  float Difference(RegionType targetRegion, RegionType sourceRegion) const
  {
    // Crop the source region to look like the potentially cropped query region. We must do this before we crop the target region.
    sourceRegion = ITKHelpers::CropRegionAtPosition(sourceRegion, this->MaskImage->GetLargestPossibleRegion(), targetRegion);

    targetRegion.Crop(this->MaskImage->GetLargestPossibleRegion());

//    std::cout << "GMHDifference targetRegion: " << targetRegion << std::endl;
//    std::cout << "GMHDifference sourceRegion: " << sourceRegion << std::endl;
    assert(this->Image->GetLargestPossibleRegion() == this->MaskImage->GetLargestPossibleRegion());
    assert(this->Image->GetLargestPossibleRegion().IsInside(targetRegion));
    assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    // Define some types
    typedef float BinValueType; // bins must be float if we are going to normalize
    typedef MaskedHistogramGenerator<BinValueType> MaskedHistogramGeneratorType;
    typedef HistogramGenerator<BinValueType> HistogramGeneratorType;
    typedef HistogramGeneratorType::HistogramType HistogramType;

    // Setup storage of the gradients
    typedef itk::Image<itk::CovariantVector<float, 2>, 2> GradientImageType;

    // Extract the channels and compute their derivatives
    typedef GradientImageType::PixelType::RealValueType ScalarType;

    typedef itk::NthElementImageAdaptor<TImage, ScalarType> ImageChannelAdaptorType;
    typename ImageChannelAdaptorType::Pointer imageChannelAdaptor = ImageChannelAdaptorType::New();
    imageChannelAdaptor->SetImage(this->Image);

    // Initialize the final histograms. The channel histograms will be concatenated to form these final histograms.
    HistogramType targetHistogram;
    HistogramType sourceHistogram;

    // These are reused in the loop to store the gradients of a single channel at a time
    GradientImageType::Pointer channelGradients = GradientImageType::New();

    // Connect the gradients to their Norm adaptors. We use this filter to compute the magnitude of the gradient images
    typedef itk::NormImageAdaptor<GradientImageType, ScalarType>
        NormImageAdaptorType;
    typename NormImageAdaptorType::Pointer normImageAdaptor = NormImageAdaptorType::New();


    // Compute the gradient of each channel
    for(unsigned int channel = 0; channel < 3; ++channel) // 3 is the number of RGB channels
    {
      imageChannelAdaptor->SelectNthElement(channel);
      Derivatives::MaskedGradientInRegion(imageChannelAdaptor.GetPointer(), this->MaskImage,
                                          imageChannelAdaptor->GetLargestPossibleRegion(),
                                          channelGradients.GetPointer());

      // This has to be set here after channelGradients has been set to the correct size in the gradient function.
      // I don't understand why this has to be the case... the TestChangeImage() in ITKHelpers/Tests/TestNormImageAdaptor.cpp
      // indicates that the adaptors size is updated automatically when the image size changes.
      normImageAdaptor->SetImage(channelGradients.GetPointer());

      std::vector<itk::Index<2> > validPixels =
          ITKHelpers::GetPixelsWithValueInRegion(this->MaskImage, targetRegion,
                                                 this->MaskImage->GetValidValue());

      // Get the valid pixels
      std::vector<ScalarType> targetGradientMagnitudeValues =
          ITKHelpers::GetPixelValues(normImageAdaptor.GetPointer(), validPixels);

      // Get the range of the valid pixels
      ScalarType minTargetGradientMagnitude = Helpers::Min(targetGradientMagnitudeValues);
      ScalarType maxTargetGradientMagnitude = Helpers::Max(targetGradientMagnitudeValues);

      // Compute histograms of the gradient magnitudes (to measure texture)
      bool allowOutside = false; // The histogram range should be fixed at the target range.
      HistogramType targetChannelHistogram =
        MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
            normImageAdaptor.GetPointer(), targetRegion,
            this->MaskImage, targetRegion, this->NumberOfBinsPerChannel,
            minTargetGradientMagnitude, maxTargetGradientMagnitude,
            allowOutside, this->MaskImage->GetValidValue());

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
            normImageAdaptor.GetPointer(), sourceRegion,
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
  TImage* Image;

  Mask* MaskImage;

  unsigned int NumberOfBinsPerChannel;
};

#endif
