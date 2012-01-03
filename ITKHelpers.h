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

#ifndef ITKHELPERS_H
#define ITKHELPERS_H

// Custom
#include "Patch.h"
#include "Types.h"
class Mask;

// VTK
class vtkImageData;
class vtkPolyData;

namespace ITKHelpers
{

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Non-template function declarations (defined in Helpers.cpp) ///////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

// Return the highest value of the specified image out of the pixels under a specified BoundaryImage.
itk::Index<2> FindHighestValueInMaskedRegion(const FloatScalarImageType* image, float& maxValue, UnsignedCharScalarImageType* maskImage);

itk::ImageRegion<2> CropToRegion(const itk::ImageRegion<2>& inputRegion, const itk::ImageRegion<2>& targetRegion);

// Look from a pixel across the hole in a specified direction and return the pixel that exists on the other side of the hole.
itk::Index<2> FindPixelAcrossHole(const itk::Index<2>& queryPixel, const FloatVector2Type& direction, const Mask* const mask);

void OutputImageType(const itk::ImageBase<2>* input);

// Average each component of a list of vectors then construct and return a new vector composed of these averaged components.
FloatVector2Type AverageVectors(const std::vector<FloatVector2Type>& vectors);

// Set the center pixel of a 'region' in an 'image' to the specified 'color'. The region is assumed to have odd dimensions.
void SetRegionCenterPixel(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char color[3]);

// This function creates an Offset<2> by setting the 'dimension' index of the Offset<2> to the value of the Offset<1>, and filling the other position with a 0.
itk::Offset<2> OffsetFrom1DOffset(const itk::Offset<1>& offset1D, const unsigned int dimension);

// Convert an RGB image to the CIELAB colorspace.
void RGBImageToCIELabImage(RGBImageType* const rgbImage, FloatVectorImageType* cielabImage);

// Convert the first 3 channels of an ITK image to the CIELAB colorspace.
void ITKImageToCIELabImage(const FloatVectorImageType* rgbImage, FloatVectorImageType* cielabImage);

// Normalize every pixel/vector in a vector image.
void NormalizeVectorImage(FloatVector2ImageType* image);

// Compute the angle between two vectors.
float AngleBetween(const FloatVector2Type v1, const FloatVector2Type v2);

// This function simply drives ITKImagetoVTKRGBImage or ITKImagetoVTKMagnitudeImage based on the number of components of the input.
void ITKVectorImageToVTKImageFromDimension(const FloatVectorImageType* image, vtkImageData* outputImage);

// These functions create a VTK image from a multidimensional ITK image.
void ITKImageToVTKRGBImage(const FloatVectorImageType* image, vtkImageData* outputImage);
void ITKImageToVTKMagnitudeImage(const FloatVectorImageType* image, vtkImageData* outputImage);
void ITKImageChannelToVTKImage(const FloatVectorImageType* image, const unsigned int channel, vtkImageData* outputImage);

// Create a VTK image of a patch of an image.
void CreatePatchVTKImage(const FloatVectorImageType* image, const itk::ImageRegion<2>& region, vtkImageData* outputImage);

// Create a VTK image filled with values representing vectors. (There is no concept of a "vector image" in VTK).
void ITKImageToVTKVectorFieldImage(const FloatVector2ImageType* image, vtkImageData* outputImage);

// Convert the first 3 channels of a float vector image to an unsigned char/color/rgb image.
void VectorImageToRGBImage(const FloatVectorImageType* image, RGBImageType* rgbImage);

// Get the center pixel of a region. The region is assumed to have odd dimensions.
itk::Index<2> GetRegionCenter(const itk::ImageRegion<2>& region);

// Get the size of a region by it's radius. E.g. radius 5 -> 11x11 patch.
itk::Size<2> SizeFromRadius(const unsigned int radius);

// It is too intensive to glyph every vector in a vector image. In many cases, the vector field may have
// very large regions of zero vectors. This function creates the vectors for only the non-zero pixels in
// the vector image.
void ConvertNonZeroPixelsToVectors(const FloatVector2ImageType* vectorImage, vtkPolyData* output);

// "Follow" a vector from one pixel to find the next pixel it would "hit".
itk::Index<2> GetNextPixelAlongVector(const itk::Index<2>& pixel, const FloatVector2Type& vector);

// Get the direction in integer pixel coordinates of the 'vector'
itk::Offset<2> GetOffsetAlongVector(const FloatVector2Type& vector);

// Make an ImageRegion centered on 'pixel' with radius 'radius'.
itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2>& pixel, const unsigned int radius);

// Apply the MaskedBlur function to every channel of a VectorImage separately.
void VectorMaskedBlur(const FloatVectorImageType* inputImage, const Mask* mask, const float blurVariance, FloatVectorImageType* output);

// Simply calls OutlineRegion followed by BlankRegion
void BlankAndOutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3]);

// Set pixels on the boundary of 'region' in 'image' to 'value'.
void OutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3]);

// Set all pixels in 'region' in 'image' to black.
void BlankRegion(vtkImageData* image, const itk::ImageRegion<2>& region);

// Get the offsets of the 8 neighborhood of a pixel.
std::vector<itk::Offset<2> > Get8NeighborOffsets();

void DeepCopy(const itk::ImageBase<2>* input, itk::ImageBase<2>* output);

// The return value MUST be a smart pointer
itk::ImageBase<2>::Pointer CreateImageWithSameType(const itk::ImageBase<2>* input);

/////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Template function declarations (defined in ITKHelpers.hxx) ///////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


// Determine if any of the 8 neighbors pixels has the specified value.
template<typename TImage>
bool HasNeighborWithValue(const itk::Index<2>& pixel, const TImage* const image, const typename TImage::PixelType& value);

template<typename TVectorImage>
void AnisotropicBlurAllChannels(const TVectorImage* image, TVectorImage* output, const float sigma);

template<typename TImage>
void OutlineRegion(TImage* image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value);

template<typename TImage>
void DeepCopy(const TImage* input, TImage* output);

// Note: specialization declarations must appear in the header or the compiler does not know about their definition in the .cpp file!
template<>
void DeepCopy<FloatVectorImageType>(const FloatVectorImageType* input, FloatVectorImageType* output);

template<typename TImage>
void DeepCopyInRegion(const TImage* input, const itk::ImageRegion<2>& region, TImage* output);

template <typename TImage>
void ITKScalarImageToScaledVTKImage(const TImage* image, vtkImageData* outputImage);

template <class TImage>
void CopyPatch(const TImage* sourceImage, TImage* targetImage, const itk::Index<2>& sourcePosition, const itk::Index<2>& targetPosition, const unsigned int radius);

template <class TImage>
void CopyPatch(const TImage* sourceImage, TImage* targetImage, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

template <class TImage>
void CreateConstantPatch(TImage* patch, const typename TImage::PixelType& value, const unsigned int radius);

template<typename TImage>
void ReplaceValue(TImage* image, const typename TImage::PixelType& queryValue, const typename TImage::PixelType& replacementValue);

template <class TImage>
void CopyPatchIntoImage(const TImage* patch, TImage* image, const itk::Index<2>& position);


template <class TImage>
void CopySelfPatchIntoHoleOfTargetRegion(TImage* image, const Mask* mask,
                                   const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);

template <class TImage>
void CopySourcePatchIntoHoleOfTargetRegion(const TImage* sourceImage, TImage* targetImage, const Mask* mask,
                                           const itk::ImageRegion<2>& sourceRegionInput, const itk::ImageRegion<2>& destinationRegionInput);

template <class TImage>
float MaxValue(const TImage* image);

template <class T>
std::vector<T> MaxValuesVectorImage(const itk::VectorImage<T, 2>* const image);

template <class TImage>
float MaxValueLocation(const TImage* image);

template <class TImage>
float MinValue(const TImage* image);


template <class TImage>
itk::Index<2> MinValueLocation(const TImage* image);

template <typename TImage>
void ColorToGrayscale(const TImage* colorImage, UnsignedCharScalarImageType* grayscaleImage);

// template <typename TPixelType>
// float PixelSquaredDifference(const TPixelType&, const TPixelType&);

template<typename TImage>
void BlankAndOutlineRegion(TImage* image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& blankValue, const typename TImage::PixelType& outlineValue);

template<typename TImage>
void SetRegionToConstant(TImage* image, const itk::ImageRegion<2>& region,const typename TImage::PixelType& constant);

template<typename TImage>
void SetImageToConstant(TImage* image, const typename TImage::PixelType& constant);

template<typename TImage>
unsigned int CountNonZeroPixels(const TImage* image);

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const TImage* image);

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const TImage* image, const itk::ImageRegion<2>& region);

template <typename TImage>
void MaskedBlur(const TImage* inputImage, const Mask* mask, const float blurVariance, TImage* output);

template<typename TImage>
void InitializeImage(TImage* image, const itk::ImageRegion<2>& region);

template<typename TImage>
void CreatePatchImage(TImage* image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, const Mask* mask, TImage* result);


template<typename TImage>
void DilateImage(const TImage* image, TImage* dilatedImage, const unsigned int radius);

template<typename TImage>
void ChangeValue(const TImage* image, const typename TImage::PixelType& oldValue, const typename TImage::PixelType& newValue);

template<typename TPixel>
void ExtractChannel(const itk::VectorImage<TPixel, 2>* image, const unsigned int channel, typename itk::Image<TPixel, 2>* output);

template<typename TPixel>
void ScaleChannel(const itk::VectorImage<TPixel, 2>* image, const unsigned int channel, const TPixel channelMax, typename itk::VectorImage<TPixel, 2>* output);

template<typename TPixel>
void ReplaceChannel(const itk::VectorImage<TPixel, 2>* image, const unsigned int channel, typename itk::Image<TPixel, 2>* replacement, typename itk::VectorImage<TPixel, 2>* output);

template<typename TImage>
typename TImage::TPixel ComputeMaxPixelDifference(const TImage* image);

template<typename TImage>
void ReadImage(const std::string&, TImage* image);

template<typename TInputImage, typename TOutputImage, typename TFilter>
void FilterImage(const TInputImage* input, TOutputImage* output);

// Median filter an image ignoring a masked region.
template<typename TImage>
void MaskedMedianFilter(const TImage* inputImage, const Mask* mask, const unsigned int kernelRadius, TImage* output);

// template<typename TImage>
// void ApplyToAllChannels(typename TImage::Pointer image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value);

}// end namespace

#include "ITKHelpers.hxx"

#endif