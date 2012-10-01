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

#ifndef LinearSearchBestLidarTextureDerivatives_HPP
#define LinearSearchBestLidarTextureDerivatives_HPP

// Submodules
#include <Utilities/Histogram/HistogramHelpers.hpp>
#include <Utilities/Histogram/HistogramDifferences.hpp>
#include <Utilities/Histogram/HistogramGenerator.hpp>

#include <ITKHelpers/itkNormImageAdaptor.h>

// Custom
#include "ImageProcessing/Derivatives.h"
#include <Utilities/Debug/Debug.h>

// ITK
#include "itkNthElementImageAdaptor.h"
#include "itkAbsImageAdaptor.h"

/**
 * This class uses comparisons of histograms of the derivatives
 * (absolute values, as left vs right is irrelevant/arbitrary)
 * to determine the best match
 * from a list of patches (normally supplied by a KNN search using an SSD criterion).
 *
 * This function template is similar to std::min_element but can be used when the comparison
 * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the
 * the element in the range [first,last) which has the "smallest" distance (of course, both the
 * distance metric and comparison can be overriden to perform something other than the canonical
 * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
 * \tparam DistanceValueType The value-type for the distance measures.
 * \tparam DistanceFunctionType The functor type to compute the distance measure.
 * \tparam CompareFunctionType The functor type that can compare two distance measures (strict weak-ordering).
 */
template <typename PropertyMapType, typename TImage, typename TIterator, typename TImageToWrite = TImage>
class LinearSearchBestLidarTextureDerivatives : public Debug
{
  PropertyMapType PropertyMap;
  TImage* Image;
  Mask* MaskImage;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestLidarTextureDerivatives(PropertyMapType propertyMap, TImage* const image, Mask* const mask) :
    Debug(), PropertyMap(propertyMap), Image(image), MaskImage(mask)
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
    unsigned int numberOfBins = 30;

    // If the input element range is empty, there is nothing to do.
    if(first == last)
    {
      std::cerr << "LinearSearchBestHistogram: Nothing to do..." << std::endl;
      return *last;
    }

    // Get the region to process
    itk::ImageRegion<2> queryRegion = get(this->PropertyMap, query).GetRegion();

//    typedef itk::Image<float, 2> ImageChannelType;
    typedef itk::NthElementImageAdaptor<TImage, float> ImageChannelAdaptorType;
    typename ImageChannelAdaptorType::Pointer imageChannelAdaptor = ImageChannelAdaptorType::New();
    imageChannelAdaptor->SetImage(this->Image);

    typedef itk::Image<itk::CovariantVector<float, 2>, 2> GradientImageType;

    // Setup storage for the gradient of each channel
    std::vector<GradientImageType::Pointer> imageChannelGradients(this->Image->GetNumberOfComponentsPerPixel());

    // Compute the gradient of each RGB channel
    for(unsigned int channel = 0; channel < 3; ++channel)
    {
      imageChannelAdaptor->SelectNthElement(channel);

      if(this->DebugImages)
      {
        std::stringstream ssImageFile;
        ssImageFile << "ImageChannel_" << channel << ".mha";
        ITKHelpers::WriteImage(imageChannelAdaptor.GetPointer(), ssImageFile.str());
      }

      // Compute gradients
      GradientImageType::Pointer gradientImage = GradientImageType::New();
      gradientImage->SetRegions(this->Image->GetLargestPossibleRegion()); // We allocate the full image because in the next loop we will compute gradients in all of the source patch regions.
      gradientImage->Allocate();

      Derivatives::MaskedGradientInRegion(imageChannelAdaptor.GetPointer(), this->MaskImage,
                                          queryRegion, gradientImage.GetPointer());

      if(this->DebugImages)
      {
        std::stringstream ssGradientFile;
        ssGradientFile << "GradientImage_" << channel << ".mha";
        ITKHelpers::WriteImage(gradientImage.GetPointer(), ssGradientFile.str());
      }

      imageChannelGradients[channel] = gradientImage;
    }

//    typedef int BinValueType;
    typedef float BinValueType; // bins must be float if we are going to normalize
    typedef MaskedHistogramGenerator<BinValueType> MaskedHistogramGeneratorType;
    typedef HistogramGenerator<BinValueType> HistogramGeneratorType;
    typedef HistogramGeneratorType::HistogramType HistogramType;

    HistogramType targetHistogram;

    // Store, for each RGB channel (the elements of the vector), the min/max x/y derivative value of the valid region of the target patch
    typedef std::vector<GradientImageType::PixelType::RealValueType> ValueVectorType;

    std::vector<ValueVectorType> minRGBChannelDerivativeMagnitudes(2); // x and y
    for(unsigned int i = 0; i < 2; ++i)
    {
      minRGBChannelDerivativeMagnitudes[i].resize(3);
    }

    std::vector<ValueVectorType> maxRGBChannelDerivativeMagnitudes(2); // x and y
    for(unsigned int i = 0; i < 2; ++i)
    {
      maxRGBChannelDerivativeMagnitudes[i].resize(3);
    }

    // Store, for each depth derivative channel (elements 0 is Dx, element 1 is Dy), the min/max value
    typedef std::vector<GradientImageType::PixelType::RealValueType> ValueVectorType;

    ValueVectorType minDepthChannelDerivativeMagnitudes(2); // Dx and Dy

    ValueVectorType maxDepthChannelDerivativeMagnitudes(2); // Dx and Dy

    std::vector<itk::Index<2> > validPixels = ITKHelpers::GetPixelsWithValueInRegion(this->MaskImage, queryRegion, this->MaskImage->GetValidValue());

    typedef itk::NthElementImageAdaptor<GradientImageType, float> GradientChannelAdaptorType;
    typename GradientChannelAdaptorType::Pointer gradientChannelAdaptor = GradientChannelAdaptorType::New();

    typedef itk::AbsImageAdaptor<GradientChannelAdaptorType, GradientImageType::PixelType::RealValueType> AbsImageAdaptorType;
    typename AbsImageAdaptorType::Pointer absImageAdaptor = AbsImageAdaptorType::New();

    // Compute the gradient magnitude images for each RGB channel's gradient, and compute the histograms for the target/query region
    for(unsigned int channel = 0; channel < 3; ++channel)
    {
      gradientChannelAdaptor->SetImage(imageChannelGradients[channel].GetPointer());
      for(unsigned int gradientChannel = 0; gradientChannel < 2; ++gradientChannel)
      {
        gradientChannelAdaptor->SelectNthElement(gradientChannel);

        absImageAdaptor->SetImage(gradientChannelAdaptor.GetPointer());

        std::vector<GradientImageType::PixelType::RealValueType> derivativeMagnitudes =
            ITKHelpers::GetPixelValues(absImageAdaptor.GetPointer(), validPixels);

        minRGBChannelDerivativeMagnitudes[gradientChannel][channel] = Helpers::Min(derivativeMagnitudes);
        maxRGBChannelDerivativeMagnitudes[gradientChannel][channel] = Helpers::Max(derivativeMagnitudes);

        // Compute histograms of the gradient magnitudes (to measure texture)
        bool allowOutside = false;
        HistogramType targetChannelDerivativeHistogram =
          MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
              absImageAdaptor.GetPointer(), queryRegion, this->MaskImage, queryRegion, numberOfBins,
              minRGBChannelDerivativeMagnitudes[gradientChannel][channel],
              maxRGBChannelDerivativeMagnitudes[gradientChannel][channel],
              allowOutside, this->MaskImage->GetValidValue());

        targetHistogram.Append(targetChannelDerivativeHistogram);
      }
    }

    targetHistogram.Normalize();

    // Compute the histograms of the depth derivative channels in the target/query region
    HistogramType targetDepthDerivativesHistogram;
    typedef itk::AbsImageAdaptor<ImageChannelAdaptorType, GradientImageType::PixelType::RealValueType> DepthDerivativeAbsImageAdaptorType;
    typename DepthDerivativeAbsImageAdaptorType::Pointer depthDerivativeAbsImageAdaptor = DepthDerivativeAbsImageAdaptorType::New();
    for(unsigned int derivativeChannel = 0; derivativeChannel < 2; ++derivativeChannel)
    {
      imageChannelAdaptor->SelectNthElement(derivativeChannel + 3);

      depthDerivativeAbsImageAdaptor->SetImage(imageChannelAdaptor.GetPointer());

      std::vector<GradientImageType::PixelType::RealValueType> derivativeMagnitudes =
          ITKHelpers::GetPixelValues(depthDerivativeAbsImageAdaptor.GetPointer(), validPixels);

      minDepthChannelDerivativeMagnitudes[derivativeChannel] = Helpers::Min(derivativeMagnitudes);
      maxDepthChannelDerivativeMagnitudes[derivativeChannel] = Helpers::Max(derivativeMagnitudes);

      // Compute histograms of the gradient magnitudes (to measure texture)
      bool allowOutside = false;
      HistogramType targetChannelDerivativeHistogram =
        MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
            depthDerivativeAbsImageAdaptor.GetPointer(), queryRegion, this->MaskImage, queryRegion, numberOfBins,
            minDepthChannelDerivativeMagnitudes[derivativeChannel],
            maxDepthChannelDerivativeMagnitudes[derivativeChannel],
            allowOutside, this->MaskImage->GetValidValue());

      targetDepthDerivativesHistogram.Append(targetChannelDerivativeHistogram);
    }

    // Separately normalize the depth derivative histograms, then append them to the RGB histtograms
    targetDepthDerivativesHistogram.Normalize();
    targetHistogram.Append(targetDepthDerivativesHistogram);

    // Initialize
    float bestDistance = std::numeric_limits<float>::max();
    TIterator bestPatch = last;

    unsigned int bestId = 0; // Keep track of which of the top SSD patches is the best by histogram score (just for information sake)
    HistogramType bestHistogram;

    // Iterate through all of the input elements
    for(TIterator currentPatch = first; currentPatch != last; ++currentPatch)
    {
      itk::ImageRegion<2> currentRegion = get(this->PropertyMap, *currentPatch).GetRegion();

      HistogramType testHistogram;

      for(unsigned int channel = 0; channel < 3; ++channel)
      {
        gradientChannelAdaptor->SetImage(imageChannelGradients[channel].GetPointer());

        Derivatives::MaskedGradientInRegion(imageChannelAdaptor.GetPointer(), this->MaskImage,
                                            currentRegion, imageChannelGradients[channel].GetPointer());

        for(unsigned int gradientChannel = 0; gradientChannel < 2; ++gradientChannel)
        {
          gradientChannelAdaptor->SelectNthElement(gradientChannel);

          absImageAdaptor->SetImage(gradientChannelAdaptor.GetPointer());

          // Compute the histogram of the source region using the queryRegion mask
          bool allowOutside = true;
          // Compare to the valid region of the source patch
          //        HistogramType testChannelHistogram =
          //            MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
          //                        imageChannelGradientMagnitudes[channel].GetPointer(), currentRegion, this->MaskImage, queryRegion, numberOfBins,
          //                        minChannelGradientMagnitudes[channel], maxChannelGradientMagnitudes[channel], allowOutside, this->MaskImage->GetValidValue());

          // Compare to the hole region of the source patch
          //        HistogramType testChannelHistogram =
          //            MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
          //                        imageChannelGradientMagnitudes[channel].GetPointer(), currentRegion, this->MaskImage, queryRegion, numberOfBins,
          //                        minChannelGradientMagnitudes[channel], maxChannelGradientMagnitudes[channel], allowOutside, this->MaskImage->GetHoleValue());

          // Compare to the entire source patch (by passing the source region as the mask region, which is entirely valid)
          //        HistogramType testChannelHistogram =
          //            MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
          //                        normImageAdaptor.GetPointer(), currentRegion, this->MaskImage, currentRegion, numberOfBins,
          //                        minChannelGradientMagnitudes[channel], maxChannelGradientMagnitudes[channel], allowOutside, this->MaskImage->GetValidValue());

          // We don't need a masked histogram since we are using the full source patch
          HistogramType testChannelHistogram = HistogramGeneratorType::ComputeScalarImageHistogram(
                gradientChannelAdaptor.GetPointer(), currentRegion,
                numberOfBins,
                minRGBChannelDerivativeMagnitudes[gradientChannel][channel],
                maxRGBChannelDerivativeMagnitudes[gradientChannel][channel], allowOutside);

          testHistogram.Append(testChannelHistogram);
        } // end gradientChannel loop
      } // end image channel loop

      testHistogram.Normalize();

      // Compute the histograms of the depth derivative channels
      HistogramType testDepthDerivativesHistogram;
      typedef itk::AbsImageAdaptor<ImageChannelAdaptorType, GradientImageType::PixelType::RealValueType> DepthDerivativeAbsImageAdaptorType;
      typename DepthDerivativeAbsImageAdaptorType::Pointer depthDerivativeAbsImageAdaptor = DepthDerivativeAbsImageAdaptorType::New();
      for(unsigned int derivativeChannel = 0; derivativeChannel < 2; ++derivativeChannel)
      {
        imageChannelAdaptor->SelectNthElement(derivativeChannel + 3);

        depthDerivativeAbsImageAdaptor->SetImage(imageChannelAdaptor.GetPointer());

        // Compute histograms of the gradient magnitudes (to measure texture)
        bool allowOutside = true;
        HistogramType testChannelDerivativeHistogram = HistogramGeneratorType::ComputeScalarImageHistogram(
              gradientChannelAdaptor.GetPointer(), currentRegion,
              numberOfBins,
              minDepthChannelDerivativeMagnitudes[derivativeChannel],
              maxDepthChannelDerivativeMagnitudes[derivativeChannel], allowOutside);

        testDepthDerivativesHistogram.Append(testChannelDerivativeHistogram);
      }

      // Separately normalize the depth derivative histograms, then append them to the RGB histtograms
      testDepthDerivativesHistogram.Normalize();
      testHistogram.Append(testDepthDerivativesHistogram);

      float histogramDifference = HistogramDifferences::HistogramDifference(targetHistogram, testHistogram);

      if(this->DebugOutputs)
      {
        std::cout << "histogramDifference " << currentPatch - first << " : " << histogramDifference << std::endl;
      }

      if(histogramDifference < bestDistance)
      {
        bestDistance = histogramDifference;
        bestPatch = currentPatch;

        // These are not needed - just for debugging
        bestId = currentPatch - first;
        bestHistogram = testHistogram;
      }
    }

    std::cout << "BestId: " << bestId << std::endl;
    std::cout << "Best distance: " << bestDistance << std::endl;

//    this->Iteration++;

    return *bestPatch;
  }

}; // end class LinearSearchBestHistogramDifference

#endif
