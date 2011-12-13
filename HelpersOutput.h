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

#ifndef HELPERS_OUTPUT_H
#define HELPERS_OUTPUT_H

#include <vtkImageData.h>
#include <vtkPolyData.h>

#include "Mask.h"
#include "Patch.h"
#include "Types.h"

#include "itkRescaleIntensityImageFilter.h"

namespace HelpersOutput
{
  
// Write a vtkPolyData to a .vtp file.
void WritePolyData(const vtkPolyData* polyData, const std::string& fileName);

// Write a vtkImageData to a .vti file.
void WriteImageData(const vtkImageData* imageData, const std::string& fileName);


// Paraview requires 3D vectors to display glyphs, even if the vectors are really 2D. These functions appends a 0 to each vectors of a 2D vector image so that it can be easily visualized with Paraview.
void Write2DVectorRegion(const FloatVector2ImageType::Pointer image, const itk::ImageRegion<2>& region, const std::string& filename);

// Calls Write2DVectorRegion on a full image.
void Write2DVectorImage(const FloatVector2ImageType::Pointer image, const std::string& filename);

// Write the first 3 channels of a FloatVectorImageType as an unsigned char (RGB) image.
void WriteVectorImageAsRGB(const FloatVectorImageType::Pointer image, const std::string& fileName);

////////////////////////////////////
///////// Function templates (defined in HelpersOutput.hxx) /////////
////////////////////////////////////

template<typename T>
void WriteRGBImage(const typename T::Pointer input, const std::string& filename);

template <typename TDebugImageType>
void WriteSequentialImage(const typename TDebugImageType::Pointer image, const std::string& filePrefix, const unsigned int iteration);

template <typename TDebugImageType>
void WriteImageConditional(const typename TDebugImageType::Pointer image, const std::string& fileName, const bool condition);

template <class T>
void WriteScaledScalarImage(const typename T::Pointer image, const std::string& filename);

template<typename T>
void WriteImage(const typename T::Pointer image, const std::string& filename);

template<typename TImage>
void WritePatch(const typename TImage::Pointer image, const Patch& patch, const std::string& filename);

template<typename TImage>
void WriteMaskedPatch(const typename TImage::Pointer image, const Mask::Pointer mask, const Patch& patch, const std::string& filename);

template<typename TImage>
void WriteMaskedRegion(const typename TImage::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region, const std::string& filename);

template<typename TImage>
void WriteRegion(const typename TImage::Pointer image, const itk::ImageRegion<2>& region, const std::string& filename);

} // end namespace

#include "HelpersOutput.hxx"

#endif
