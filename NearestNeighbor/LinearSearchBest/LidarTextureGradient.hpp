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

#ifndef LinearSearchBestLidarTextureGradient_HPP
#define LinearSearchBestLidarTextureGradient_HPP

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

/**
 * This class uses comparisons of histograms of the gradient magnitudes to determine the best match
 * from a list of patches (normally supplied by a KNN search using an SSD criterion).
 *
 * This class expects an RGBDxDy image to be passed.
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
class LinearSearchBestLidarTextureGradient : public Debug
{
  PropertyMapType PropertyMap;
  TImage* Image;
  Mask* MaskImage;
  unsigned int Iteration = 0;
  TImageToWrite* ImageToWrite = nullptr;

  typedef itk::Image<itk::CovariantVector<float, 2>, 2> GradientImageType;
  std::vector<GradientImageType::Pointer> RGBChannelGradients;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestLidarTextureGradient(PropertyMapType propertyMap, TImage* const image, Mask* const mask, TImageToWrite* imageToWrite = nullptr) :
    Debug(), PropertyMap(propertyMap), Image(image), MaskImage(mask), ImageToWrite(imageToWrite)
  {
    // Compute the gradients in all source patches
    typedef itk::NthElementImageAdaptor<TImage, float> ImageChannelAdaptorType;
    typename ImageChannelAdaptorType::Pointer imageChannelAdaptor = ImageChannelAdaptorType::New();
    imageChannelAdaptor->SetImage(this->Image);

    RGBChannelGradients.resize(3);

    for(unsigned int channel = 0; channel < 3; ++channel) // 3 RGB channels
    {
      imageChannelAdaptor->SelectNthElement(channel);

      GradientImageType::Pointer gradientImage = GradientImageType::New();
      gradientImage->SetRegions(this->Image->GetLargestPossibleRegion());
      gradientImage->Allocate();

      Derivatives::MaskedGradient(imageChannelAdaptor.GetPointer(), this->MaskImage, gradientImage.GetPointer());

      RGBChannelGradients[channel] = gradientImage;
    }
  }

  struct RegionSorter
  {
    itk::Functor::IndexLexicographicCompare<2> IndexCompareFunctor;
    bool operator()(const itk::ImageRegion<2> region1, const itk::ImageRegion<2> region2) const
    {
      return IndexCompareFunctor(region1.GetIndex(), region2.GetIndex());
    }
  };

  typedef float BinValueType; // bins must be float if we are going to normalize
  typedef MaskedHistogramGenerator<BinValueType> MaskedHistogramGeneratorType;
  typedef HistogramGenerator<BinValueType> HistogramGeneratorType;
  typedef HistogramGeneratorType::HistogramType HistogramType;

  typedef std::map<itk::ImageRegion<2>, HistogramType, RegionSorter> HistogramMapType;
  HistogramMapType PreviouslyComputedRGBHistograms;
  HistogramMapType PreviouslyComputedDepthHistograms;

  bool WritePatches = false;

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  typename TIterator::value_type operator()(const TIterator first, const TIterator last, typename TIterator::value_type query)
  {
    if(WritePatches)
    {
      if(this->ImageToWrite == nullptr)
      {
        throw std::runtime_error("LinearSearchBestLidarTextureGradient cannot WriteTopPatches without having an ImageToWrite!");
      }
      PatchHelpers::WriteTopPatches(this->ImageToWrite, this->PropertyMap, first, last,
                                    "BestPatches", this->Iteration);
    }

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

    // Compute the gradient of each channel
    for(unsigned int channel = 0; channel < 3; ++channel) // 3 is the number of RGB channels
    {
      imageChannelAdaptor->SelectNthElement(channel);

      Derivatives::MaskedGradientInRegion(imageChannelAdaptor.GetPointer(), this->MaskImage,
                                          queryRegion, this->RGBChannelGradients[channel].GetPointer());

      if(this->DebugImages)
      {
        std::stringstream ssGradientFile;
        ssGradientFile << "GradientImage_" << channel << ".mha";
        ITKHelpers::WriteImage(this->RGBChannelGradients[channel].GetPointer(), ssGradientFile.str());
      }
    }

    HistogramType targetRGBHistogram;

    // Store, for each channel (the elements of the vector), the min/max value of the valid region of the target patch
    std::vector<GradientImageType::PixelType::RealValueType> minRGBChannelGradientMagnitudes(this->Image->GetNumberOfComponentsPerPixel());
    std::vector<GradientImageType::PixelType::RealValueType> maxRGBChannelGradientMagnitudes(this->Image->GetNumberOfComponentsPerPixel());

    std::vector<itk::Index<2> > validPixels = ITKHelpers::GetPixelsWithValueInRegion(this->MaskImage, queryRegion, this->MaskImage->GetValidValue());

    typedef itk::NormImageAdaptor<GradientImageType, GradientImageType::PixelType::RealValueType> NormImageAdaptorType;
    typename NormImageAdaptorType::Pointer normImageAdaptor = NormImageAdaptorType::New();

    // Compute the gradient magnitude images for each RGB channel's gradient, and compute the histograms for the target/query region
    for(unsigned int channel = 0; channel < 3; ++channel) // 3 is the number of RGB channels
    {
      imageChannelAdaptor->SelectNthElement(channel);

      normImageAdaptor->SetImage(this->RGBChannelGradients[channel].GetPointer());

      std::vector<GradientImageType::PixelType::RealValueType> gradientMagnitudes =
          ITKHelpers::GetPixelValues(normImageAdaptor.GetPointer(), validPixels);

      minRGBChannelGradientMagnitudes[channel] = Helpers::Min(gradientMagnitudes);
      maxRGBChannelGradientMagnitudes[channel] = Helpers::Max(gradientMagnitudes);

      // Compute histograms of the gradient magnitudes (to measure texture)
      bool allowOutside = false;
      HistogramType targetRGBChannelHistogram =
        MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
            normImageAdaptor.GetPointer(), queryRegion, this->MaskImage, queryRegion, numberOfBins,
            minRGBChannelGradientMagnitudes[channel], maxRGBChannelGradientMagnitudes[channel],
            allowOutside, this->MaskImage->GetValidValue());

      targetRGBChannelHistogram.Normalize();

      targetRGBHistogram.Append(targetRGBChannelHistogram);
    }

    // Compute the gradient magnitude from the depth derivatives, and compute the histogram for the target/query region
    // Extract the depth derivative channels
    std::vector<unsigned int> depthDerivativeChannels = {3,4};
    typedef itk::Image<itk::CovariantVector<float, 2> > DepthDerivativesImageType;
    DepthDerivativesImageType::Pointer depthDerivatives = DepthDerivativesImageType::New();
    ITKHelpers::ExtractChannels(this->Image, depthDerivativeChannels, depthDerivatives.GetPointer());

    // Compute the depth gradient magnitude image
    typedef itk::Image<float, 2> DepthGradientMagnitudeImageType;
    DepthGradientMagnitudeImageType::Pointer depthGradientMagnitude = DepthGradientMagnitudeImageType::New();
    ITKHelpers::MagnitudeImage(depthDerivatives.GetPointer(), depthGradientMagnitude.GetPointer());

    // Store the min/max values (histogram range)
    std::vector<DepthGradientMagnitudeImageType::PixelType> depthGradientMagnitudes =
        ITKHelpers::GetPixelValues(depthGradientMagnitude.GetPointer(), validPixels);

    float minDepthChannelGradientMagnitude = Helpers::Min(depthGradientMagnitudes);
    float maxDepthChannelGradientMagnitude = Helpers::Max(depthGradientMagnitudes);

    // Compute histogram
    bool allowOutsideForDepthHistogramCreation = false;
    HistogramType targetDepthHistogram =
      MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
          depthGradientMagnitude.GetPointer(), queryRegion, this->MaskImage, queryRegion, numberOfBins,
          minDepthChannelGradientMagnitude, maxDepthChannelGradientMagnitude,
          allowOutsideForDepthHistogramCreation, this->MaskImage->GetValidValue());

    targetDepthHistogram.Normalize();

    // Initialize
    float bestDistance = std::numeric_limits<float>::max();
    TIterator bestPatch = last;

    unsigned int bestId = 0; // Keep track of which of the top SSD patches is the best by histogram score (just for information sake)
    HistogramType bestRGBHistogram;
    HistogramType bestDepthHistogram;

    // Iterate through all of the supplied source patches
    for(TIterator currentPatch = first; currentPatch != last; ++currentPatch)
    {
      itk::ImageRegion<2> currentRegion = get(this->PropertyMap, *currentPatch).GetRegion();

      // Determine if the gradient and histogram have already been computed
      typename HistogramMapType::iterator histogramMapIterator;
      histogramMapIterator = this->PreviouslyComputedRGBHistograms.find(currentRegion);

      bool alreadyComputed;

      if(histogramMapIterator == this->PreviouslyComputedRGBHistograms.end())
      {
        alreadyComputed = false;
      }
      else
      {
        alreadyComputed = true;
      }

      HistogramType testRGBHistogram;

      bool allowOutside = true;
      // Compute the RGB histograms of the source region using the queryRegion mask
      for(unsigned int channel = 0; channel < 3; ++channel) // 3 is the number of RGB channels
      {
        normImageAdaptor->SetImage(this->RGBChannelGradients[channel].GetPointer());

        HistogramType testRGBChannelHistogram;

        if(!alreadyComputed)
        {
          // We don't need a masked histogram since we are using the full source patch
          testRGBChannelHistogram = HistogramGeneratorType::ComputeScalarImageHistogram(
                          normImageAdaptor.GetPointer(), currentRegion,
                          numberOfBins,
                          minRGBChannelGradientMagnitudes[channel],
                          maxRGBChannelGradientMagnitudes[channel], allowOutside);

          testRGBChannelHistogram.Normalize();

          this->PreviouslyComputedRGBHistograms[currentRegion] = testRGBChannelHistogram;
        }
        else // already computed
        {
          testRGBChannelHistogram = this->PreviouslyComputedRGBHistograms[currentRegion];
        }


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


        testRGBHistogram.Append(testRGBChannelHistogram);
      }

      HistogramType testDepthHistogram;

      if(!alreadyComputed)
      {
        // Compute the depth histogram of the source region using the queryRegion mask
        testDepthHistogram = HistogramGeneratorType::ComputeScalarImageHistogram(
                        depthGradientMagnitude.GetPointer(), currentRegion,
                        numberOfBins,
                        minDepthChannelGradientMagnitude,
                        maxDepthChannelGradientMagnitude, allowOutside);

        testDepthHistogram.Normalize();

        this->PreviouslyComputedDepthHistograms[currentRegion] = testDepthHistogram;
      }
      else
      {
        testDepthHistogram = this->PreviouslyComputedDepthHistograms[currentRegion];
      }

      // Compute the differences in the histograms
      float rgbHistogramDifference = HistogramDifferences::HistogramDifference(targetRGBHistogram, testRGBHistogram);
      float depthHistogramDifference = HistogramDifferences::HistogramDifference(targetDepthHistogram, testDepthHistogram);

      // Weight the depth histogram 3x so that it is a 1:1 weighting of RGB and depth difference
      float histogramDifference = rgbHistogramDifference + 3.0f * depthHistogramDifference;

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
        bestRGBHistogram = testRGBHistogram;
        bestDepthHistogram = testDepthHistogram;
      }
    }

    std::cout << "BestId: " << bestId << std::endl;
    std::cout << "Best distance: " << bestDistance << std::endl;

    this->Iteration++;

    return *bestPatch;
  }

}; // end class LinearSearchBestHistogramDifference

#endif
