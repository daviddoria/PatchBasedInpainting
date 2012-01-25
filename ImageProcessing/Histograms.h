/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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

#ifndef HISTOGRAMS_H
#define HISTOGRAMS_H

#include <vector>

//#include "Mask.h"
class Mask;
#include "Types.h"

namespace Histograms
{

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegion(const FloatVectorImageType* image, const itk::ImageRegion<2>& region, const unsigned int numberOfBins);

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegionManual(const FloatVectorImageType* image, const itk::ImageRegion<2>& region, const unsigned int numberOfBins);

std::vector<float> Compute1DHistogramOfMultiChannelImage(const FloatVectorImageType* image, const itk::ImageRegion<2>& region, const unsigned int numberOfBins);

std::vector<float> Compute1DHistogramOfMultiChannelMaskedImage(const FloatVectorImageType* image, const itk::ImageRegion<2>& imageRegion, Mask* mask, const itk::ImageRegion<2>& maskRegion, const unsigned int numberOfBins);

std::vector<HistogramType::Pointer> ComputeHistogramsOfMaskedRegion(const FloatVectorImageType* image, const itk::ImageRegion<2>& imageRegion, const Mask* mask, const itk::ImageRegion<2>& maskRegion, const unsigned int numberOfBins);

void OutputHistogram(const HistogramType::Pointer);

float HistogramDifference(const HistogramType::Pointer, const HistogramType::Pointer);

float HistogramIntersection(const HistogramType::Pointer, const HistogramType::Pointer);
float HistogramIntersection(const std::vector<float>& histogram1, const std::vector<float>& histogram2);

float NDHistogramDifference(const HistogramType::Pointer, const HistogramType::Pointer);

HistogramType::Pointer ComputeNDHistogramOfRegionManual(const FloatVectorImageType* image, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension);

HistogramType::Pointer ComputeNDHistogramOfMaskedRegionManual(const FloatVectorImageType* image, const Mask* mask, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension);

void WriteHistogram(const std::vector<float>& histogram1, const std::string& filename);

} // end namespace

#endif
