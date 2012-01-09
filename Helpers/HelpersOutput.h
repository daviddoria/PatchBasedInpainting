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

// VTK
//#include <vtkImageData.h>
//#include <vtkPolyData.h>
class vtkImageData;
class vtkPolyData;

// Custom
//#include "Mask.h"
class Mask;
#include "Patch.h"
#include "Types.h"

namespace HelpersOutput
{

// Write a vtkPolyData to a .vtp file.
void WritePolyData(vtkPolyData* const polyData, const std::string& fileName);

// Write a vtkImageData to a .vti file.
void WriteImageData(vtkImageData* const imageData, const std::string& fileName);


// Paraview requires 3D vectors to display glyphs, even if the vectors are really 2D. These functions appends a 0 to each vectors of a 2D vector image so that it can be easily visualized with Paraview.
void Write2DVectorRegion(const FloatVector2ImageType* const image, const itk::ImageRegion<2>& region, const std::string& filename);

// Calls Write2DVectorRegion on a full image.
void Write2DVectorImage(const FloatVector2ImageType* const image, const std::string& filename);

// Write the first 3 channels of a FloatVectorImageType as an unsigned char (RGB) image.
void WriteVectorImageAsRGB(const FloatVectorImageType* const image, const std::string& fileName);

////////////////////////////////////
///////// Function templates (defined in HelpersOutput.hxx) /////////
////////////////////////////////////

template<typename TImage>
void WriteRGBImage(const TImage* const input, const std::string& filename);

template <typename TImage>
void WriteSequentialImage(const TImage* const image, const std::string& filePrefix, const unsigned int iteration);

template <typename TImage>
void WriteImageConditional(const TImage* const image, const std::string& fileName, const bool condition);

template <class TImage>
void WriteScaledScalarImage(const TImage* const image, const std::string& filename);

template<typename TImage>
void WriteImage(const TImage* const image, const std::string& filename);

template<typename TImage>
void WritePatch(const TImage* const image, const Patch& patch, const std::string& filename);

template<typename TImage>
void WriteMaskedPatch(const TImage* const image, const Mask* mask, const Patch& patch, const std::string& filename);

template<typename TImage>
void WriteMaskedRegion(const TImage* const image, const Mask* mask, const itk::ImageRegion<2>& region, const std::string& filename);

template<typename TImage>
void WriteRegion(const TImage* const image, const itk::ImageRegion<2>& region, const std::string& filename);

} // end namespace

#include "HelpersOutput.hxx"

#endif
