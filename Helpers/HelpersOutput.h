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
class vtkImageData;
class vtkPolyData;

// Custom
class Mask;
#include "Types.h"

namespace HelpersOutput
{

/** Write a vtkPolyData to a .vtp file. */
void WritePolyData(vtkPolyData* const polyData, const std::string& fileName);

/** Write a vtkImageData to a .vti file. */
void WriteImageData(vtkImageData* const imageData, const std::string& fileName);


/** Paraview requires 3D vectors to display glyphs, even if the vectors are really 2D.
    These functions appends a 0 to each vectors of a 2D vector image so that it can be easily visualized with Paraview. */
void Write2DVectorRegion(const FloatVector2ImageType* const image, const itk::ImageRegion<2>& region, const std::string& filename);

/**  Calls Write2DVectorRegion on a full image. */
void Write2DVectorImage(const FloatVector2ImageType* const image, const std::string& filename);

/**  Write the first 3 channels of a FloatVectorImageType as an unsigned char (RGB) image. */
void WriteVectorImageAsRGB(const FloatVectorImageType* const image, const std::string& fileName);

////////////////////////////////////
///////// Function templates (defined in HelpersOutput.hxx) /////////
////////////////////////////////////

/**  Write the first 3 channels of an image to a file as unsigned chars. */
template<typename TImage>
void WriteRGBImage(const TImage* const input, const std::string& filename);

/** Write an image to a file named 'prefix'_'iteration'.mha*/
template <typename TImage>
void WriteSequentialImage(const TImage* const image, const std::string& filePrefix, const unsigned int iteration);

/** Write 'image' to 'fileName' if 'condition' is true. */
template <typename TImage>
void WriteImageConditional(const TImage* const image, const std::string& fileName, const bool condition);

/** Scale a scalar image to the range (0-255) and then write it to a file as an unsigned char image. */
template <class TImage>
void WriteScaledScalarImage(const TImage* const image, const std::string& filename);

/** Write 'image' to 'fileName'.*/
template<typename TImage>
void WriteImage(const TImage* const image, const std::string& fileName);

/** Write a 'region' of an 'image' to 'filename', coloring any invalid pixels in 'mask' the color 'holeColor'. */
template<typename TImage>
void WriteMaskedRegion(const TImage* const image, const Mask* mask, const itk::ImageRegion<2>& region, const std::string& filename,
                       const typename TImage::PixelType& holeColor);

/** Write a 'region' of an 'image' to 'filename'.*/
template<typename TImage>
void WriteRegion(const TImage* const image, const itk::ImageRegion<2>& region, const std::string& filename);

} // end namespace

// These functions should go somewhere else. We don't want any Helpers* to be rebuilt based on an API change.
///** Write an ImagePatch as an image to 'fileName'. */
//template<typename TImage>
//void WriteImagePatch(const ImagePatch<TImage>& patch, const std::string& fileName);

///** Write an ImagePatch as an image to 'fileName', coloring all invalid pixels in 'mask' the color 'holeColor'. */
//template<typename TImage>
//void WriteMaskedImagePatch(const Mask* mask, const ImagePatch<TImage>& patch, const std::string& fileName, const typename TImage::PixelType& holeColor);

#include "HelpersOutput.hxx"

#endif
