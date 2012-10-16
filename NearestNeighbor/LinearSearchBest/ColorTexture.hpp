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

#ifndef LinearSearchBestColorTexture_HPP
#define LinearSearchBestColorTexture_HPP

#include <Utilities/Histogram/HistogramHelpers.hpp>
#include <Utilities/Histogram/HistogramDifferences.hpp>

#include "ImageProcessing/Derivatives.h"

#include <Utilities/Debug/Debug.h>

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
template <typename PropertyMapType, typename TImage, typename TIterator, typename TImageToWrite = TImage>
class LinearSearchBestColorTexture : public Debug
{
  PropertyMapType PropertyMap;
  TImage* Image;
  Mask* MaskImage;
  TImageToWrite* ImageToWrite;
  unsigned int Iteration = 0;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestColorTexture(PropertyMapType propertyMap, TImage* const image, Mask* const mask, TImageToWrite* const imageToWrite) :
    Debug(), PropertyMap(propertyMap), Image(image), MaskImage(mask), ImageToWrite(imageToWrite)
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

    typedef itk::Image<float, 2> ImageChannelType;

    typedef itk::Image<itk::CovariantVector<float, 2>, 2> GradientImageType;

    // Decompose the image into channels and compute the gradient of each channel
    std::vector<ImageChannelType::Pointer> imageChannels(this->Image->GetNumberOfComponentsPerPixel());
    std::vector<GradientImageType::Pointer> imageChannelGradients(this->Image->GetNumberOfComponentsPerPixel());
    for(unsigned int channel = 0; channel < this->Image->GetNumberOfComponentsPerPixel(); ++channel)
    {
      ImageChannelType::Pointer imageChannel = ImageChannelType::New();

      ITKHelpers::ExtractChannel(this->Image, channel, imageChannel.GetPointer());
      imageChannels[channel] = imageChannel;

      if(this->DebugImages)
      {
        std::stringstream ssImageFile;
        ssImageFile << "ImageChannel_" << channel << ".mha";
        ITKHelpers::WriteImage(imageChannel.GetPointer(), ssImageFile.str());
      }

      // Compute gradients
      GradientImageType::Pointer gradientImage = GradientImageType::New();
      gradientImage->SetRegions(this->Image->GetLargestPossibleRegion());
      gradientImage->Allocate();

      Derivatives::MaskedGradientInRegion(imageChannel.GetPointer(), this->MaskImage,
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
    typedef HistogramGenerator<BinValueType>::HistogramType HistogramType;

    HistogramType targetHistogram;

    typedef itk::Image<float, 2> GradientMagnitudeImageType;

    std::vector<GradientMagnitudeImageType::PixelType> minChannelGradientMagnitudes(this->Image->GetNumberOfComponentsPerPixel());
    std::vector<GradientMagnitudeImageType::PixelType> maxChannelGradientMagnitudes(this->Image->GetNumberOfComponentsPerPixel());


    std::vector<itk::Index<2> > validPixels = ITKHelpers::GetPixelsWithValueInRegion(this->MaskImage, queryRegion, this->MaskImage->GetValidValue());

    std::vector<GradientMagnitudeImageType::Pointer> imageChannelGradientMagnitudes(this->Image->GetNumberOfComponentsPerPixel());

    for(unsigned int channel = 0; channel < this->Image->GetNumberOfComponentsPerPixel(); ++channel)
    {
      GradientMagnitudeImageType::Pointer gradientMagnitudeImage = GradientMagnitudeImageType::New();
      gradientMagnitudeImage->SetRegions(this->Image->GetLargestPossibleRegion());
      gradientMagnitudeImage->Allocate();

      imageChannelGradientMagnitudes[channel] = gradientMagnitudeImage;

      ITKHelpers::MagnitudeImageInRegion(imageChannelGradients[channel].GetPointer(), queryRegion, gradientMagnitudeImage.GetPointer());
//      ITKHelpers::WriteImage(imageChannelGradients[channel].GetPointer(), "GradientMagnitudeImage.mha");

      std::vector<GradientMagnitudeImageType::PixelType> gradientMagnitudes = ITKHelpers::GetPixelValues(gradientMagnitudeImage.GetPointer(), validPixels);

      minChannelGradientMagnitudes[channel] = Helpers::Min(gradientMagnitudes);
      maxChannelGradientMagnitudes[channel] = Helpers::Max(gradientMagnitudes);

      // Compute histograms of the gradient magnitudes (to measure texture)
      bool allowOutside = false;
      HistogramType targetChannelHistogram =
        MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
            gradientMagnitudeImage.GetPointer(), queryRegion, this->MaskImage, queryRegion, numberOfBins,
            minChannelGradientMagnitudes[channel], maxChannelGradientMagnitudes[channel], allowOutside, this->MaskImage->GetValidValue());

      targetHistogram.Append(targetChannelHistogram);
    }

    targetHistogram.Normalize();

    if(this->DebugOutputFiles)
    {
      targetHistogram.Write(Helpers::GetSequentialFileName("TargetHistogram", this->Iteration, "txt", 3));
    }

    // Initialize
    float bestDistance = std::numeric_limits<float>::max();
    TIterator bestPatch = last;

    unsigned int bestId = 0; // Keep track of which of the top SSD patches is the best by histogram score (just for information sake)
    HistogramType bestHistogram;


    // Store the scores in this container so we can sort them later
    std::vector<float> scores(last - first);

    // Iterate through all of the input elements
    for(TIterator currentPatch = first; currentPatch != last; ++currentPatch)
    {
      itk::ImageRegion<2> currentRegion = get(this->PropertyMap, *currentPatch).GetRegion();

      HistogramType testHistogram;

      for(unsigned int channel = 0; channel < this->Image->GetNumberOfComponentsPerPixel(); ++channel)
      {
        Derivatives::MaskedGradientInRegion(imageChannels[channel].GetPointer(), this->MaskImage,
                                            currentRegion, imageChannelGradients[channel].GetPointer());
        ITKHelpers::MagnitudeImageInRegion(imageChannelGradients[channel].GetPointer(), currentRegion, imageChannelGradientMagnitudes[channel].GetPointer());
//        ITKHelpers::WriteImage(gradientMagnitudeImage.GetPointer(), "GradientMagnitudeImage.mha");

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
        HistogramType testChannelHistogram =
            MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
                        imageChannelGradientMagnitudes[channel].GetPointer(), currentRegion, this->MaskImage, currentRegion, numberOfBins,
                        minChannelGradientMagnitudes[channel], maxChannelGradientMagnitudes[channel], allowOutside, this->MaskImage->GetValidValue());

        testHistogram.Append(testChannelHistogram);
      }

      testHistogram.Normalize();

      if(this->DebugOutputFiles)
      {
        unsigned int patchId = currentPatch - first;
        std::stringstream ss;
        ss << "TestHistogram_" << Helpers::ZeroPad(this->Iteration, 3) << "_" << Helpers::ZeroPad(patchId, 3) << ".txt";
        testHistogram.Write(ss.str());
      }

      float histogramDifference = HistogramDifferences::HistogramDifference(targetHistogram, testHistogram);

      scores[currentPatch - first] = histogramDifference;

      if(this->DebugScreenOutputs)
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

    if(this->DebugImages)
    {
      if(this->ImageToWrite == nullptr)
      {
        throw std::runtime_error("LinearSearchBestHistogramDifference cannot WriteTopPatches without having an ImageToWrite!");
      }

      typedef ParallelSort<float> ParallelSortType;

      ParallelSortType::IndexedVector sortedScores = ParallelSortType::ParallelSortAscending(scores);

      std::vector<typename TIterator::value_type> sortedPatches(last - first);

      for(unsigned int i = 0; i < sortedPatches.size(); ++i)
      {
        unsigned int currentId = sortedScores[i].index;

        TIterator current = first;
        std::advance(current, currentId);

        sortedPatches[i] = *current;
//        std::cout << "Set sortedPatches " << i << " to " << currentId << std::endl;
      }

      unsigned int gridWidth = 10;
      unsigned int gridHeight = 10;
      PatchHelpers::WriteTopPatchesGrid(this->ImageToWrite, this->PropertyMap, sortedPatches.begin(), sortedPatches.end(),
                                        "BestPatchesSorted", this->Iteration, gridWidth, gridHeight);
    }

    this->Iteration++;

    return *bestPatch;
  }

}; // end class LinearSearchBestHistogramDifference

#endif
