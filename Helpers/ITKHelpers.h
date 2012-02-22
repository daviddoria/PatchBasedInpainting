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

#ifndef ITKHelpers_H
#define ITKHelpers_H

// Custom
class Mask;
#include "Utilities/TypeTraits.h"

// STL
#include <string>

// ITK
#include "itkImage.h"
#include "itkIndex.h"
#include "itkRGBPixel.h"
#include "itkSize.h"
#include "itkVectorImage.h"

namespace ITKHelpers
{

/** Some useful types. */
typedef itk::Image<float, 2> FloatScalarImageType;
typedef itk::Image<unsigned char, 2> UnsignedCharScalarImageType;
typedef itk::CovariantVector<float, 2> FloatVector2Type;
typedef itk::Image<itk::RGBPixel<unsigned char>, 2> RGBImageType;
typedef itk::VectorImage<float, 2> FloatVectorImageType;

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Non-template function declarations (defined in Helpers.cpp) ///////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

/** Get the number of components per pixel in an image file. */
unsigned int GetNumberOfComponentsPerPixelInFile(const std::string& filename);

/** Get a short string of an itk::Index */
std::string GetIndexString(const itk::Index<2>& index);

/** Get a short string of an itk::Size */
std::string GetSizeString(const itk::Size<2>& size);

itk::ImageRegion<2> CropToRegion(const itk::ImageRegion<2>& inputRegion, const itk::ImageRegion<2>& targetRegion);

void OutputImageType(const itk::ImageBase<2>* const input);

/** Average each component of a list of vectors then construct and return a new vector composed of these averaged components. */
FloatVector2Type AverageVectors(const std::vector<FloatVector2Type>& vectors);

/** This function creates an Offset<2> by setting the 'dimension' index of the Offset<2>
 * to the value of the Offset<1>, and filling the other position with a 0. */
itk::Offset<2> OffsetFrom1DOffset(const itk::Offset<1>& offset1D, const unsigned int dimension);

/** Convert an RGB image to the CIELAB colorspace. 'rgbImage' cannot be const because the adaptor doesn't allow it. */
void RGBImageToCIELabImage(RGBImageType* const rgbImage, FloatVectorImageType* const cielabImage);

/** Convert the first 3 channels of an ITK image to the CIELAB colorspace. */
void ITKImageToCIELabImage(const FloatVectorImageType* const rgbImage, FloatVectorImageType* const cielabImage);

/** Compute the angle between two vectors. */
float AngleBetween(const FloatVector2Type& v1, const FloatVector2Type& v2);

/** Convert the first 3 channels of a float vector image to an unsigned char/color/rgb image. */
void VectorImageToRGBImage(const FloatVectorImageType* const image, RGBImageType* const rgbImage);

/** Get the center pixel of a region. The region is assumed to have odd dimensions. */
itk::Index<2> GetRegionCenter(const itk::ImageRegion<2>& region);

/** Get the size of a region by it's radius. E.g. radius 5 -> 11x11 patch. */
itk::Size<2> SizeFromRadius(const unsigned int radius);

/** "Follow" a vector from one pixel to find the next pixel it would "hit". */
itk::Index<2> GetNextPixelAlongVector(const itk::Index<2>& pixel, const FloatVector2Type& vector);

/** Get the direction in integer pixel coordinates of the 'vector' */
itk::Offset<2> GetOffsetAlongVector(const FloatVector2Type& vector);

/** Make an ImageRegion centered on 'pixel' with radius 'radius'. */
itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2>& pixel, const unsigned int radius);

/** Get the offsets of the 8 neighborhood of a pixel. */
std::vector<itk::Offset<2> > Get8NeighborOffsets();

/** Get the indices of the neighbors of a pixel that are inside of a region. */
std::vector<itk::Index<2> > Get8NeighborsInRegion(const itk::ImageRegion<2>& region, const itk::Index<2>& pixel);

//void DeepCopyUnknownType(const itk::ImageBase<2>* input, itk::ImageBase<2>* output);

/** The return value MUST be a smart pointer. */
itk::ImageBase<2>::Pointer CreateImageWithSameType(const itk::ImageBase<2>* input);

itk::VariableLengthVector<float> Average(const std::vector<itk::VariableLengthVector<float> >& v);

std::vector<itk::Index<2> > OffsetsToIndices(const std::vector<itk::Offset<2> >& offsets, const itk::Index<2>& index);

std::vector<itk::Index<2> > GetBoundaryPixels(const itk::ImageRegion<2>& region);

/////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Template function declarations (defined in ITKHelpers.hxx) ///////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


/** Determine if any of the 8 neighbors pixels has the specified value. */
template<typename TImage>
bool HasNeighborWithValue(const itk::Index<2>& pixel, const TImage* const image, const typename TImage::PixelType& value);

/** Blur all channels of an image. */
template<typename TVectorImage>
void AnisotropicBlurAllChannels(const TVectorImage* const image, TVectorImage* const output, const float sigma);

/** Set the values of the pixels on the boundary of the 'region' to 'value'. */
template<typename TImage>
void OutlineRegion(TImage* image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value);

/** Deep copy an image. */
template<typename TImage>
void DeepCopy(const TImage* const input, TImage* const output);

/** Deep copy a vector image.
 * Note: specialization declarations must appear in the header or the compiler does
 * not know about their definition in the .cpp file!
 */
template<>
void DeepCopy<FloatVectorImageType>(const FloatVectorImageType* const input, FloatVectorImageType* const output);

template<typename TImage>
void DeepCopyInRegion(const TImage* const input, const itk::ImageRegion<2>& region, TImage* const output);

template <class TImage>
void CopyRegion(const TImage* const sourceImage, TImage* const targetImage, const itk::Index<2>& sourcePosition,
                const itk::Index<2>& targetPosition, const unsigned int radius);

template <class TImage>
void CopyRegion(const TImage* const sourceImage, TImage* const targetImage, const itk::ImageRegion<2>& sourceRegion,
               const itk::ImageRegion<2>& targetRegion);

template <class TImage>
void CopySelfRegion(TImage* const image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

template<typename TImage>
void ReplaceValue(TImage* image, const typename TImage::PixelType& queryValue, const typename TImage::PixelType& replacementValue);

template <class TImage>
void CopyRegionIntoImage(const TImage* const patch, TImage* const image, const itk::Index<2>& position);

template <class TImage>
float MaxValue(const TImage* const image);

template <class T>
std::vector<T> MaxValuesVectorImage(const itk::VectorImage<T, 2>* const image);

template <class TImage>
float MaxValueLocation(const TImage* const image);

template <class TImage>
float MinValue(const TImage* const image);

template <class TImage>
itk::Index<2> MinValueLocation(const TImage* const image);

template <typename TInputImage, typename TOutputImage>
void ColorToGrayscale(const TInputImage* const colorImage, TOutputImage* const grayscaleImage);

template<typename TImage>
void BlankAndOutlineRegion(TImage* const image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& blankValue,
                           const typename TImage::PixelType& outlineValue);

template<typename TImage>
void SetRegionToConstant(TImage* const image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& constant);

template<typename TImage>
void SetImageToConstant(TImage* const image, const typename TImage::PixelType& constant);

template<typename TImage>
unsigned int CountNonZeroPixels(const TImage* const image);

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const TImage* const image);

template<typename TImage>
std::vector<itk::Index<2> > GetNonZeroPixels(const TImage* const image, const itk::ImageRegion<2>& region);

template<typename TImage>
void InitializeImage(TImage* const image, const itk::ImageRegion<2>& region);

/** This function will be used/deduced when called with itk::VectorImage<float,2> image; InitializeImage(image, region) */
template<typename TImage>
void InitializeImage(const itk::VectorImage<TImage>* const input, const itk::ImageRegion<2>& region);

template<typename TImage>
void DilateImage(const TImage* const image, TImage* const dilatedImage, const unsigned int radius);

/** Change the value of all pixels with value = 'oldValue' to 'newValue. */
template<typename TImage>
void ChangeValue(const TImage* const image, const typename TImage::PixelType& oldValue,
                 const typename TImage::PixelType& newValue);

/** Extract a channel of an image. */
template<typename TPixel>
void ExtractChannel(const itk::VectorImage<TPixel, 2>* const image, const unsigned int channel,
                    typename itk::Image<TPixel, 2>* const output);

/** Scale a channel of an image between 0 and 'channelMax'. */
template<typename TPixel>
void ScaleChannel(const itk::VectorImage<TPixel, 2>* const image, const unsigned int channel,
                  const TPixel channelMax, typename itk::VectorImage<TPixel, 2>* const output);

/** Replace a channel of an image. */
template<typename TPixel>
void ReplaceChannel(const itk::VectorImage<TPixel, 2>* const image, const unsigned int channel,
                    typename itk::Image<TPixel, 2>* const replacement,
                    typename itk::VectorImage<TPixel, 2>* const output);

/** Read an image from a file. */
template<typename TImage>
void ReadImage(const std::string&, TImage* const image);

/** TODO: apply any filter to an image. */
template<typename TInputImage, typename TOutputImage, typename TFilter>
void FilterImage(const TInputImage* const input, TOutputImage* const output);

/** Normalize every pixel/vector in a vector image. */
template<typename TImage>
void NormalizeVectorImage(TImage* const image);

/** Create an itk::Index<2> from any object with a operator[]. */
template<typename T>
itk::Index<2> CreateIndex(const T& v);

/** Get the average value of the neighbors of a pixel. */
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType AverageNeighborValue(const TImage* const image, const itk::Index<2>& pixel);

template<typename TImage>
std::vector<itk::Index<2> > Get8NeighborsWithValue(const itk::Index<2>& pixel, const TImage* const image,
                                                   const typename TImage::PixelType& value);

/** Compute the average of the values appearing at the specified indices. */
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType AverageOfPixelsAtIndices(const TImage* const image, const std::vector<itk::Index<2> >& indices);

/** Compute the variance of the values appearing at the specified indices. The variance of the ith component is the
 * ith component of the output pixel*/
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType VarianceOfPixelsAtIndices(const TImage* const image, const std::vector<itk::Index<2> >& indices);

/** Compute the average of all pixels in a region.*/
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType AverageInRegion(const TImage* const image, const itk::ImageRegion<2>& region);

/** Compute the average of all pixels in a region.*/
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType VarianceInRegion(const TImage* const image, const itk::ImageRegion<2>& region);

/** Compute the average difference of corresponding pixels from two regions of an image.*/
template<typename TImage, typename TDifferenceFunctor>
float AverageDifferenceInRegion(const TImage* const image, const itk::ImageRegion<2>& region1,
                                const itk::ImageRegion<2>& region2, TDifferenceFunctor differenceFunctor);

/** Compute the average difference of corresponding pixels from regions in two images.*/
template<typename TImage, typename TDifferenceFunctor>
float AverageDifferenceInRegion(const TImage* const image1, const itk::ImageRegion<2>& region1,
                                const TImage* const image2, const itk::ImageRegion<2>& region2,
                                TDifferenceFunctor differenceFunctor);

// template <typename T>
// T SumOfComponents(const itk::VariableLengthVector<T>& v);

/** Sum the componets of an object.*/
template <typename T>
float SumOfComponents(const T& v);

/** Return the length of the vector through the same interface that we have defined for std::vector and scalars in Helpers.*/
template<typename T>
unsigned int length(const itk::VariableLengthVector<T>& v);

/** Return the specified component of the vector using the same interface that we have defined for std::vector and scalars in Helpers.*/
template<typename T>
T& index(itk::VariableLengthVector<T>& v, size_t i);

template<typename T>
T index(const itk::VariableLengthVector<T>& v, size_t i);

template<typename T>
void SetObjectToZero(T& object);

}// end namespace

#include "ITKHelpers.hxx"

#endif
