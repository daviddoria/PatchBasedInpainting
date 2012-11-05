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

#ifndef SortByRGBTextureGradient_HPP
#define SortByRGBTextureGradient_HPP

// Submodules
#include <Utilities/Histogram/HistogramHelpers.hpp>
#include <Utilities/Histogram/HistogramDifferences.hpp>
#include <Utilities/Histogram/HistogramGenerator.h>
#include <Utilities/Histogram/MaskedHistogramGenerator.h>

#include <ITKHelpers/itkNormImageAdaptor.h>

#include <Helpers/ParallelSort.h>

// Custom
#include "ImageProcessing/Derivatives.h"
#include <Utilities/Debug/Debug.h>

// ITK
#include "itkNthElementImageAdaptor.h"

/**
 * This class uses comparisons of histograms of the gradient magnitudes to determine the best match
 * from a list of patches (normally supplied by a KNN search using an SSD criterion).
 *
 * This class expects an RGB image to be passed.
 *
 * This algorithm will search for the
 * the element in the range [first,last) which has the "smallest" distance (of course, both the
 * distance metric and comparison can be overriden to perform something other than the canonical
 * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
 *
 * The difference between this class and LinearSearchBestLidarHSVTextureGradient is that it not only finds
 * the top patch, but sorts the input patches according to this criteria (so that we can write a grid/list
 * of the patches in the new order for demonstrative purposes).
 */
template <typename PropertyMapType, typename TImage, typename TImageToWrite = TImage>
class SortByRGBTextureGradient : public Debug
{
  PropertyMapType PropertyMap;
  TImage* Image;
  Mask* MaskImage;
  unsigned int Iteration = 0;
  TImageToWrite* ImageToWrite = nullptr;

  typedef itk::Image<itk::CovariantVector<float, 2>, 2> GradientImageType;
  std::vector<GradientImageType::Pointer> HSVChannelGradients;

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  SortByRGBTextureGradient(PropertyMapType propertyMap, TImage* const image, Mask* const mask,
                           TImageToWrite* imageToWrite = nullptr, const Debug& debug = Debug()) :
    Debug(debug), PropertyMap(propertyMap), Image(image), MaskImage(mask), ImageToWrite(imageToWrite)
  {
    // Compute the gradients in all source patches
    typedef itk::NthElementImageAdaptor<TImage, float> ImageChannelAdaptorType;
    typename ImageChannelAdaptorType::Pointer imageChannelAdaptor = ImageChannelAdaptorType::New();
    imageChannelAdaptor->SetImage(this->Image);

    this->HSVChannelGradients.resize(3);

    for(unsigned int channel = 0; channel < 3; ++channel) // 3 HSV channels
    {
      imageChannelAdaptor->SelectNthElement(channel);

      GradientImageType::Pointer gradientImage = GradientImageType::New();
      gradientImage->SetRegions(this->Image->GetLargestPossibleRegion());
      gradientImage->Allocate();

      if(channel == 0) // H Channel
      {
        Helpers::HSV_H_Difference hsvHDifference;
        Derivatives::MaskedGradient(imageChannelAdaptor.GetPointer(), this->MaskImage,
                                    gradientImage.GetPointer(), hsvHDifference);
      }
      else
      {
        Derivatives::MaskedGradient(imageChannelAdaptor.GetPointer(), this->MaskImage,
                                    gradientImage.GetPointer());
      }

      this->HSVChannelGradients[channel] = gradientImage;

      if(this->DebugImages)
      {
        std::stringstream ss;
        ss << "HSV_Gradient_" << channel << ".mha";
        ITKHelpers::WriteImage(this->HSVChannelGradients[channel].GetPointer(), ss.str());
      }
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
  HistogramMapType PreviouslyComputedHSVHistograms;
  HistogramMapType PreviouslyComputedDepthHistograms;

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \param outputFirst An iterator pointing to the beginning of the (preallocated) output container.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  template <typename TIterator, typename TOutputIterator>
  typename TIterator::value_type operator()(const TIterator first, const TIterator last,
                                            typename TIterator::value_type query,
                                            TOutputIterator outputFirst)
  {
    if(this->DebugImages)
    {
      if(this->ImageToWrite == nullptr)
      {
        throw std::runtime_error("LinearSearchBestLidarTextureGradient cannot WriteTopPatches without having an ImageToWrite!");
      }
//      PatchHelpers::WriteTopPatches(this->ImageToWrite, this->PropertyMap, first, last,
//                                    "BestPatches", this->Iteration);

      unsigned int gridWidth = 10;
      unsigned int gridHeight = 10;
      PatchHelpers::WriteTopPatchesGrid(this->ImageToWrite, this->PropertyMap, first, last,
                                        "BestPatches", this->Iteration, gridWidth, gridHeight);
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

    typedef itk::NthElementImageAdaptor<TImage, float> ImageChannelAdaptorType;
    typename ImageChannelAdaptorType::Pointer imageChannelAdaptor = ImageChannelAdaptorType::New();
    imageChannelAdaptor->SetImage(this->Image);

    // Compute the gradient of each channel
    for(unsigned int channel = 0; channel < 3; ++channel) // 3 is the number of HSV channels
    {
      imageChannelAdaptor->SelectNthElement(channel);

      if(channel == 0) // H channel
      {
        Helpers::HSV_H_Difference hsvHDifference;
        Derivatives::MaskedGradientInRegion(imageChannelAdaptor.GetPointer(), this->MaskImage,
                                            queryRegion, this->HSVChannelGradients[channel].GetPointer(), hsvHDifference);
      }
      else
      {
        Derivatives::MaskedGradientInRegion(imageChannelAdaptor.GetPointer(), this->MaskImage,
                                            queryRegion, this->HSVChannelGradients[channel].GetPointer());
      }

      if(this->DebugImages && this->DebugLevel > 1)
      {
        // Gradient of patch
        std::stringstream ssGradientFile;
        ssGradientFile << "QueryHSVGradient_" << Helpers::ZeroPad(this->Iteration, 3) << "_" << channel << ".mha";
        ITKHelpers::WriteRegionAsImage(this->HSVChannelGradients[channel].GetPointer(), queryRegion, ssGradientFile.str());

        // Full gradient image
//        std::stringstream ss;
//        ss << "HSV_Gradient_" << Helpers::ZeroPad(this->Iteration, 3) << "_" << channel << ".mha";
//        ITKHelpers::WriteImage(this->HSVChannelGradients[channel].GetPointer(), ss.str());
      }
    }

    HistogramType targetHSVHistogram;

    // Store, for each channel (the elements of the vector), the min/max value of the valid region of the target patch
    std::vector<GradientImageType::PixelType::RealValueType> minHSVChannelGradientMagnitudes(this->Image->GetNumberOfComponentsPerPixel());
    std::vector<GradientImageType::PixelType::RealValueType> maxHSVChannelGradientMagnitudes(this->Image->GetNumberOfComponentsPerPixel());

    std::vector<itk::Index<2> > validPixels = ITKHelpers::GetPixelsWithValueInRegion(this->MaskImage, queryRegion, this->MaskImage->GetValidValue());

    typedef itk::NormImageAdaptor<GradientImageType, GradientImageType::PixelType::RealValueType> NormImageAdaptorType;
    typename NormImageAdaptorType::Pointer normImageAdaptor = NormImageAdaptorType::New();

    // Compute the gradient magnitude images for each HSV channel's gradient, and compute the histograms for the target/query region
    for(unsigned int channel = 0; channel < 3; ++channel) // 3 is the number of HSV channels
    {
      imageChannelAdaptor->SelectNthElement(channel);

      normImageAdaptor->SetImage(this->HSVChannelGradients[channel].GetPointer());

      std::vector<GradientImageType::PixelType::RealValueType> gradientMagnitudes =
          ITKHelpers::GetPixelValues(normImageAdaptor.GetPointer(), validPixels);

      minHSVChannelGradientMagnitudes[channel] = Helpers::Min(gradientMagnitudes);
      maxHSVChannelGradientMagnitudes[channel] = Helpers::Max(gradientMagnitudes);

      // Compute histograms of the gradient magnitudes (to measure texture)
      bool allowOutside = false;
      HistogramType targetHSVChannelHistogram =
        MaskedHistogramGeneratorType::ComputeMaskedScalarImageHistogram(
            normImageAdaptor.GetPointer(), queryRegion, this->MaskImage, queryRegion, numberOfBins,
            minHSVChannelGradientMagnitudes[channel], maxHSVChannelGradientMagnitudes[channel],
            allowOutside, this->MaskImage->GetValidValue());

      targetHSVChannelHistogram.Normalize();

      targetHSVHistogram.Append(targetHSVChannelHistogram);
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

    if(this->DebugOutputFiles)
    {
      targetHSVHistogram.Write(Helpers::GetSequentialFileName("TargetHSVHistogram", this->Iteration, "txt", 3));
      targetDepthHistogram.Write(Helpers::GetSequentialFileName("TargetDepthHistogram", this->Iteration, "txt", 3));
    }

    // Initialize
    float bestDistance = std::numeric_limits<float>::max();
    TIterator bestPatch = last;

    unsigned int bestId = 0; // Keep track of which of the top SSD patches is the best by histogram score (just for information sake)
    HistogramType bestHSVHistogram;
    HistogramType bestDepthHistogram;

    // Store the scores in this container so we can sort them later
    std::vector<float> scores(last - first);

    // Iterate through all of the supplied source patches
    for(TIterator currentPatch = first; currentPatch != last; ++currentPatch)
    {
      itk::ImageRegion<2> currentRegion = get(this->PropertyMap, *currentPatch).GetRegion();

      // Determine if the gradient and histogram have already been computed
      typename HistogramMapType::iterator histogramMapIterator;
      histogramMapIterator = this->PreviouslyComputedHSVHistograms.find(currentRegion);

      bool alreadyComputed;

      if(histogramMapIterator == this->PreviouslyComputedHSVHistograms.end())
      {
        alreadyComputed = false;
      }
      else
      {
        alreadyComputed = true;
      }

      HistogramType testHSVHistogram;

      bool allowOutside = true;
      // Compute the HSV histograms of the source region using the queryRegion mask
      for(unsigned int channel = 0; channel < 3; ++channel) // 3 is the number of HSV channels
      {
        if(this->DebugImages && this->DebugLevel > 1)
        {
          std::stringstream ssSourceGradientFile;
          ssSourceGradientFile << "SourceGradient_" << Helpers::ZeroPad(this->Iteration, 3) << "_" << channel << "_" << Helpers::ZeroPad(currentPatch - first, 3) <<  ".mha";
          ITKHelpers::WriteRegionAsImage(this->HSVChannelGradients[channel].GetPointer(), currentRegion, ssSourceGradientFile.str());
        }

        normImageAdaptor->SetImage(this->HSVChannelGradients[channel].GetPointer());

        HistogramType testHSVChannelHistogram;

        if(!alreadyComputed)
        {
          // We don't need a masked histogram since we are using the full source patch
          testHSVChannelHistogram = HistogramGeneratorType::ComputeScalarImageHistogram(
                          normImageAdaptor.GetPointer(), currentRegion,
                          numberOfBins,
                          minHSVChannelGradientMagnitudes[channel],
                          maxHSVChannelGradientMagnitudes[channel], allowOutside);

          testHSVChannelHistogram.Normalize();

          this->PreviouslyComputedHSVHistograms[currentRegion] = testHSVChannelHistogram;
        }
        else // already computed
        {
          testHSVChannelHistogram = this->PreviouslyComputedHSVHistograms[currentRegion];
        }

        testHSVHistogram.Append(testHSVChannelHistogram);
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

      if(this->DebugOutputFiles)
      {
        std::stringstream ssEnding;
        ssEnding << "_" << Helpers::ZeroPad(this->Iteration, 3) << "_" << Helpers::ZeroPad(currentPatch - first, 3) << ".txt";
        testHSVHistogram.Write("TestHSVHistogram" + ssEnding.str());
        testDepthHistogram.Write("TestDepthHistogram" + ssEnding.str());
      }

      // Compute the differences in the histograms
      float hsvHistogramDifference = HistogramDifferences::HistogramDifference(targetHSVHistogram, testHSVHistogram);
      float depthHistogramDifference = HistogramDifferences::HistogramDifference(targetDepthHistogram, testDepthHistogram);

      // Weight the depth histogram 3x so that it is a 1:1 weighting of HSV and depth difference
      float histogramDifference = hsvHistogramDifference + 3.0f * depthHistogramDifference;

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
        bestHSVHistogram = testHSVHistogram;
        bestDepthHistogram = testDepthHistogram;
      }
    }

    std::cout << "BestId: " << bestId << std::endl;
    std::cout << "Best distance: " << bestDistance << std::endl;

    if(this->DebugOutputFiles)
    {
      Helpers::WriteVectorToFileLines(scores, Helpers::GetSequentialFileName("Scores", this->Iteration, "txt", 3));
    }

    typedef ParallelSort<float> ParallelSortType;

    ParallelSortType::IndexedVector sortedScores = ParallelSortType::ParallelSortAscending(scores);

    if(this->DebugImages)
    {
      if(this->ImageToWrite == nullptr)
      {
        throw std::runtime_error("LinearSearchBestLidarTextureGradient cannot WriteTopPatches without having an ImageToWrite!");
      }

      TOutputIterator currentOutputIterator = outputFirst; // c++ doesn't allow a second counter to be declared inside for()
      for(unsigned int i = 0; i < sortedScores.size(); ++i, ++currentOutputIterator)
      {
        unsigned int currentId = sortedScores[i].index;

        TIterator current = first;
        std::advance(current, currentId);

        *currentOutputIterator = *current;
//        std::cout << "Set sortedPatches " << i << " to " << currentId << std::endl;
      }

      unsigned int gridWidth = 10;
      unsigned int gridHeight = 10;
      PatchHelpers::WriteTopPatchesGrid(this->ImageToWrite, this->PropertyMap, outputFirst, outputFirst + sortedScores.size(),
                                        "BestPatchesSorted", this->Iteration, gridWidth, gridHeight);
    }

    this->Iteration++;

    return *bestPatch;
  } // end operator()

}; // end class SortByRGBTextureGradient

#endif
