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
#include "Mask.h"
#include "Patch.h"
#include "Types.h"

// Qt
#include <QImage>
class QGraphicsView;
class QTableWidget;

// VTK
class vtkImageData;
class vtkPolyData;

namespace Helpers
{
//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Non-template function declarations (defined in Helpers.cpp) ///////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

bool GetColumnIdByHeader(const QTableWidget* table, const std::string& header, int& columnId);

// Zero pad the 'iteration' and append it to the filePrefix, and add ".[fileExtension]" to the end.
// GetSequentialFileName("test", 2, "png");
// Produces "test_0002.png"
std::string GetSequentialFileName(const std::string& filePrefix, const unsigned int iteration, const std::string& fileExtension);

// Average each component of a list of vectors then construct and return a new vector composed of these averaged components.
FloatVector2Type AverageVectors(const std::vector<FloatVector2Type>& vectors);

// Write a vtkPolyData to a .vtp file.
void WritePolyData(const vtkPolyData* polyData, const std::string& fileName);

// Write a vtkImageData to a .vti file.
void WriteImageData(const vtkImageData* imageData, const std::string& fileName);

// Set the center pixel of an 'image' to the specified 'color'. The image is assumed to have odd dimensions.
void SetImageCenterPixel(vtkImageData* image, const unsigned char color[3]);

// Set the center pixel of a 'region' in an 'image' to the specified 'color'. The region is assumed to have odd dimensions.
void SetRegionCenterPixel(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char color[3]);

// Paraview requires 3D vectors to display glyphs, even if the vectors are really 2D. These functions appends a 0 to each vectors of a 2D vector image so that it can be easily visualized with Paraview.
void Write2DVectorRegion(const FloatVector2ImageType::Pointer image, const itk::ImageRegion<2>& region, const std::string& filename);
void Write2DVectorImage(const FloatVector2ImageType::Pointer image, const std::string& filename);

// This function creates an Offset<2> by setting the 'dimension' index of the Offset<2> to the value of the Offset<1>, and filling the other position with a 0.
itk::Offset<2> OffsetFrom1DOffset(const itk::Offset<1>& offset1D, const unsigned int dimension);

// Convert an RGB image to the CIELAB colorspace.
void RGBImageToCIELabImage(const RGBImageType::Pointer rgbImage, FloatVectorImageType::Pointer cielabImage);

// Normalize every pixel/vector in a vector image.
void NormalizeVectorImage(FloatVector2ImageType::Pointer image);

// Compute the angle between two vectors.
float AngleBetween(const FloatVector2Type v1, const FloatVector2Type v2);

// These functions create a VTK image from a multidimensional ITK image.
void ITKVectorImagetoVTKImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage); // This function simply drives ITKImagetoVTKRGBImage or ITKImagetoVTKMagnitudeImage based on the number of components of the input.
void ITKImagetoVTKRGBImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage);
void ITKImagetoVTKMagnitudeImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage);

// Create a VTK image filled with values representing vectors. (There is no concept of a "vector image" in VTK).
void ITKImagetoVTKVectorFieldImage(const FloatVector2ImageType::Pointer image, vtkImageData* outputImage);

// Convert the first 3 channels of a float vector image to an unsigned char/color/rgb image.
void VectorImageToRGBImage(const FloatVectorImageType::Pointer image, RGBImageType::Pointer rgbImage);
void WriteVectorImageAsRGB(const FloatVectorImageType::Pointer image, const std::string& fileName);

// Get the center pixel of a region. The region is assumed to have odd dimensions.
itk::Index<2> GetRegionCenter(const itk::ImageRegion<2>& region);

// Patch sizes are specified by radius so they always have an odd side length. The side length is (2*radius)+1
unsigned int SideLengthFromRadius(const unsigned int radius);

// Determine if any of a pixels 8 neighbors is a hole pixel.
bool HasHoleNeighbor(const itk::Index<2>& pixel, const Mask::Pointer mask);

// Set an image to black except for its border, which is set to 'color'.
void BlankAndOutlineImage(vtkImageData*, const unsigned char color[3]);

// Set an image to black.
void BlankImage(vtkImageData*);

// Extract the non-zero pixels of a "vector image" and convert them to vectors in a vtkPolyData. This is useful because glyphing a vector image is too slow to use as a visualization, 
// because it "draws" the vectors, even if they are zero length. In this code we are often interested in displaying vectors along a contour, so this is a very very small subset of a whole vector image.
void KeepNonZeroVectors(const vtkImageData* image, vtkPolyData* output);
void ConvertNonZeroPixelsToVectors(const FloatVector2ImageType::Pointer vectorImage, vtkPolyData* output);

// Convert a 'number' into a zero padded string.
// ZeroPad(5, 4); produces "0005"
std::string ZeroPad(const unsigned int number, const unsigned int rep);

// Make pixels where the 0th channel of inputImage matches 'value' transparent in the output image.
void MakeValueTransparent(const vtkImageData* inputImage, vtkImageData* outputImage, const unsigned char value);

// Make an entire image transparent.
void MakeImageTransparent(vtkImageData* image);

// Scale an image so that it fits in a QGraphicsView
QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx);

// "Follow" a vector from one pixel to find the next pixel it would "hit".
itk::Index<2> GetNextPixelAlongVector(const itk::Index<2>& pixel, const FloatVector2Type& vector);
itk::Offset<2> GetOffsetAlongVector(const FloatVector2Type& vector);

// Make an ImageRegion centered on 'pixel' with radius 'radius'.
itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2>& pixel, const unsigned int radius);

// "Ceil()", but also for negative numbers.
// RoundAwayFromZero(.2) = 1
// RoundAwayFromZero(-.2) = -1
// (Normally ceil(-.2) = 0
float RoundAwayFromZero(const float number);

// Apply the MaskedBlur function to every channel of a VectorImage separately.
void VectorMaskedBlur(const FloatVectorImageType::Pointer inputImage, const Mask::Pointer mask, const float blurVariance, FloatVectorImageType::Pointer output);

// Convert a QColor to an unsigned char[3]
void QColorToUCharColor(const QColor& color, unsigned char outputColor[3]);

// Simply calls OutlineRegion followed by BlankRegion
void BlankAndOutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3]);

// Set pixels on the boundary of 'region' in 'image' to 'value'.
void OutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3]);

// Set all pixels in 'region' in 'image' to black.
void BlankRegion(vtkImageData* image, const itk::ImageRegion<2>& region);

// Get the offsets of the 8 neighborhood of a pixel.
std::vector<itk::Offset<2> > Get8NeighborOffsets();

/////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Template function declarations (defined in Helpers.hxx) ///////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// template<typename TImage>
// void ApplyToAllChannels(typename TImage::Pointer image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value);

template<typename TVectorImage>
void BlurAllChannels(const typename TVectorImage::Pointer image, typename TVectorImage::Pointer output, const float sigma);

template<typename TImage>
void OutlineRegion(typename TImage::Pointer image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value);

template<typename TImage>
void DeepCopy(const typename TImage::Pointer input, typename TImage::Pointer output);

template<typename TImage>
void DeepCopyInRegion(const typename TImage::Pointer input, const itk::ImageRegion<2>& region, typename TImage::Pointer output);

template <typename TImage>
void ITKScalarImageToScaledVTKImage(const typename TImage::Pointer image, vtkImageData* outputImage);

template <typename TDebugImageType>
void WriteSequentialImage(const typename TDebugImageType::Pointer image, const std::string& filePrefix, const unsigned int iteration);

template <typename TDebugImageType>
void WriteImageConditional(const typename TDebugImageType::Pointer image, const std::string& fileName, const bool condition);

template <class T>
void WriteScaledScalarImage(const typename T::Pointer image, std::string filename);

template <class T>
void CopyPatch(const typename T::Pointer sourceImage, typename T::Pointer targetImage, const itk::Index<2>& sourcePosition, const itk::Index<2>& targetPosition, const unsigned int radius);

template <class T>
void CopyPatch(const typename T::Pointer sourceImage, typename T::Pointer targetImage, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

template <class T>
void CreateConstantPatch(typename T::Pointer patch, const typename T::PixelType value, const unsigned int radius);

template<typename T>
void ReplaceValue(typename T::Pointer image, const typename T::PixelType queryValue, const typename T::PixelType replacementValue);

template<typename T>
void WriteImage(const typename T::Pointer image, const std::string& filename);

template <class T>
void CopyPatchIntoImage(const typename T::Pointer patch, typename T::Pointer image, const itk::Index<2>& position);

template <class T>
void CreateBlankPatch(typename T::Pointer patch, const unsigned int radius);

template <class T>
void CopySelfPatchIntoHoleOfTargetRegion(typename T::Pointer image, const Mask::Pointer mask,
                                   const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);

template <class T>
void CopySourcePatchIntoHoleOfTargetRegion(typename T::Pointer sourceImage, typename T::Pointer targetImage, const Mask::Pointer mask,
                                           const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);

template <class T>
float MaxValue(const typename T::Pointer image);

template <class T>
float MaxValueLocation(const typename T::Pointer image);

template <class T>
float MinValue(const typename T::Pointer image);

template <class T>
unsigned int argmin(const typename std::vector<T>& vec);

template <class T>
itk::Index<2> MinValueLocation(const typename T::Pointer image);

template <typename TImage>
void ColorToGrayscale(const typename TImage::Pointer colorImage, UnsignedCharScalarImageType::Pointer grayscaleImage);

// template <typename TPixelType>
// float PixelSquaredDifference(const TPixelType&, const TPixelType&);

template<typename TImage>
void BlankAndOutlineRegion(typename TImage::Pointer image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& blankValue, const typename TImage::PixelType& outlineValue);

template<typename TImage>
void SetRegionToConstant(typename TImage::Pointer image, const itk::ImageRegion<2>& region,const typename TImage::PixelType& constant);

template<typename TImage>
void SetImageToConstant(typename TImage::Pointer image, const typename TImage::PixelType& constant);

template<typename TImage>
unsigned int CountNonZeroPixels(const typename TImage::Pointer image);

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const typename TImage::Pointer image);

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const typename TImage::Pointer image, const itk::ImageRegion<2>& region);

template<typename TImage>
void WritePatch(const typename TImage::Pointer image, const Patch& patch, const std::string& filename);

template<typename TImage>
void WriteMaskedPatch(const typename TImage::Pointer image, const Mask::Pointer mask, const Patch& patch, const std::string& filename);

template<typename TImage>
void WriteMaskedRegion(const typename TImage::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region, const std::string& filename);

template<typename TImage>
void WriteRegion(const typename TImage::Pointer image, const itk::ImageRegion<2>& region, const std::string& filename);

template <typename TImage>
QImage GetQImageColor(const typename TImage::Pointer image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageMagnitude(const typename TImage::Pointer image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageScalar(const typename TImage::Pointer image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageMasked(const typename TImage::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region);

template <typename TImage>
void MaskedBlur(const typename TImage::Pointer inputImage, const Mask::Pointer mask, const float blurVariance, typename TImage::Pointer output);

template<typename TImage>
void InitializeImage(typename TImage::Pointer image, const itk::ImageRegion<2>& region);

template<typename TImage>
void CreatePatchImage(typename TImage::Pointer image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, Mask::Pointer mask, typename TImage::Pointer result);

template<typename T>
void NormalizeVector(std::vector<T>& v);

}// end namespace

#include "Helpers.hxx"

#endif