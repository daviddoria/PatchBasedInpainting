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

#ifndef HELPERS_H
#define HELPERS_H

// Custom
#include "Patch.h"
#include "Types.h"
class Mask;

// VTK
class vtkImageData;
class vtkPolyData;

namespace Helpers
{
//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Non-template function declarations (defined in Helpers.cpp) ///////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

// Get a short string of an itk::Index
std::string GetString(const itk::Index<2>& index);

// Get a short string of an itk::Size
std::string GetString(const itk::Size<2>& size);

bool IsValidRGB(const int r, const int g, const int b);

void GetCellCenter(vtkImageData* imageData, const unsigned int cellId, double center[3]);

std::string ReplaceFileExtension(const std::string& fileName, const std::string& newExtension);

// Zero pad the 'iteration' and append it to the filePrefix, and add ".[fileExtension]" to the end.
// GetSequentialFileName("test", 2, "png");
// Produces "test_0002.png"
std::string GetSequentialFileName(const std::string& filePrefix, const unsigned int iteration, const std::string& fileExtension);

// Set the center pixel of an 'image' to the specified 'color'. The image is assumed to have odd dimensions.
void SetImageCenterPixel(vtkImageData* image, const unsigned char color[3]);

void CreateTransparentVTKImage(const itk::Size<2>& size, vtkImageData* outputImage);

// Patch sizes are specified by radius so they always have an odd side length. The side length is (2*radius)+1
unsigned int SideLengthFromRadius(const unsigned int radius);

// Set an image to black except for its border, which is set to 'color'.
void BlankAndOutlineImage(vtkImageData*, const unsigned char color[3]);

// Set an image to black.
void BlankImage(vtkImageData*);

// Extract the non-zero pixels of a "vector image" and convert them to vectors in a vtkPolyData. This is useful because glyphing a vector image is too slow to use as a visualization,
// because it "draws" the vectors, even if they are zero length. In this code we are often interested in displaying vectors along a contour, so this is a very very small subset of a whole vector image.
void KeepNonZeroVectors(vtkImageData* const image, vtkPolyData* output);

// Convert a 'number' into a zero padded string.
// ZeroPad(5, 4); produces "0005"
std::string ZeroPad(const unsigned int number, const unsigned int rep);

// Make pixels where the 0th channel of inputImage matches 'value' transparent in the output image.
void MakeValueTransparent(vtkImageData* const inputImage, vtkImageData* outputImage, const unsigned char value);

// Make an entire image transparent.
void MakeImageTransparent(vtkImageData* image);

// STL's .compare() function returns 0 when strings match, this is unintuitive.
bool StringsMatch(const std::string&, const std::string&);

// "Ceil()", but also for negative numbers.
// RoundAwayFromZero(.2) = 1
// RoundAwayFromZero(-.2) = -1
// (Normally ceil(-.2) = 0
float RoundAwayFromZero(const float number);

/////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Template function declarations (defined in Helpers.hxx) ///////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

template <class TImage>
void CreateBlankPatch(TImage* patch, const unsigned int radius);

template <class T>
unsigned int argmin(const typename std::vector<T>& vec);

template<typename T>
void NormalizeVector(std::vector<T>& v);

template<typename T>
T VectorMedian(std::vector<T> v);

// This pair of functions allows a scalar to be treated as the 0th component of a vector.
template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, T&>::type index(T& t, size_t);

template<typename T>
typename T::value_type& index(T& v, size_t i);

}// end namespace

#include "Helpers.hxx"

#endif