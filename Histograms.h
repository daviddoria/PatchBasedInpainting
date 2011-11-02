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

#include "Mask.h"
#include "Types.h"

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegion(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region);

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegionManual(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region);

std::vector<HistogramType::Pointer> ComputeHistogramsOfMaskedRegion(const FloatVectorImageType::Pointer image, Mask::Pointer mask, const itk::ImageRegion<2>& region);

void OutputHistogram(const HistogramType::Pointer);

float HistogramDifference(const HistogramType::Pointer, const HistogramType::Pointer);

float NDHistogramDifference(const HistogramType::Pointer, const HistogramType::Pointer);

HistogramType::Pointer ComputeNDHistogramOfRegionManual(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension);

HistogramType::Pointer ComputeNDHistogramOfMaskedRegionManual(const FloatVectorImageType::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension);

#endif
