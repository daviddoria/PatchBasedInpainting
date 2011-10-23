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

// ITK
#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodIterator.h"
#include "itkPasteImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// Qt
#include <QImage>
#include <QGraphicsView>

// VTK
class vtkImageData;
class vtkPolyData;

namespace Helpers
{
////////////////// Non-template function declarations (defined in Helpers.cpp) ///////////////////
void SetImageCenterPixel(vtkImageData* image, const unsigned char color[3]);

void SetRegionCenterPixel(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char color[3]);

itk::Offset<2> OffsetFrom1DOffset(const itk::Offset<1>& offset1D, const unsigned int dimension);

void RGBImageToCIELabImage(const RGBImageType::Pointer rgbImage, FloatVectorImageType::Pointer cielabImage);

void NormalizeVectorImage(FloatVector2ImageType::Pointer image);

float AngleBetween(const FloatVector2Type v1, const FloatVector2Type v2);

void ITKVectorImagetoVTKImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage); // This function simply drives ITKImagetoVTKRGBImage or ITKImagetoVTKMagnitudeImage
void ITKImagetoVTKRGBImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage);
void ITKImagetoVTKMagnitudeImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage);

void ITKImagetoVTKVectorFieldImage(const FloatVector2ImageType::Pointer image, vtkImageData* outputImage);

void VectorImageToRGBImage(const FloatVectorImageType::Pointer image, RGBImageType::Pointer rgbImage);

itk::Index<2> GetRegionCenter(const itk::ImageRegion<2>& region);

unsigned int SideLengthFromRadius(const unsigned int radius);

bool HasHoleNeighbor(const itk::Index<2>& pixel, const Mask::Pointer mask);

void BlankAndOutlineImage(vtkImageData*, const unsigned char color[3]);

void BlankImage(vtkImageData*);

void KeepNonZeroVectors(vtkImageData* image, vtkPolyData* output); // 'image' should be const, but VTK doesn't allow it

void ConvertNonZeroPixelsToVectors(const FloatVector2ImageType::Pointer vectorImage, vtkPolyData* output);

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegion(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region);

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegionManual(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region);

std::vector<HistogramType::Pointer> ComputeHistogramsOfMaskedRegion(const FloatVectorImageType::Pointer image, Mask::Pointer mask, const itk::ImageRegion<2>& region);

void OutputHistogram(const HistogramType::Pointer);

float HistogramDifference(const HistogramType::Pointer, const HistogramType::Pointer);

float NDHistogramDifference(const HistogramType::Pointer, const HistogramType::Pointer);

HistogramType::Pointer ComputeNDHistogramOfRegionManual(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension);

HistogramType::Pointer ComputeNDHistogramOfMaskedRegionManual(const FloatVectorImageType::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension);

std::string ZeroPad(const unsigned int number, const unsigned int rep);

void MakePixelsTransparent(vtkImageData* inputImage, vtkImageData* outputImage, const unsigned char value); // 'inputImage' should be const, but VTK doesn't allow it

void MakeImageTransparent(vtkImageData* image);

QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx);

itk::Index<2> GetNextPixelAlongVector(const itk::Index<2>& pixel, const FloatVector2Type& vector);

itk::Offset<2> GetOffsetAlongVector(const FloatVector2Type& vector);

itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2>& pixel, const unsigned int radius);

float RoundAwayFromZero(const float number);

void VectorMaskedBlur(const FloatVectorImageType::Pointer inputImage, const Mask::Pointer mask, const unsigned int kernelRadius, FloatVectorImageType::Pointer output);

void QColorToUCharColor(const QColor& color, unsigned char outputColor[3]);

void BlankAndOutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3]);

void OutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3]);

void BlankRegion(vtkImageData* image, const itk::ImageRegion<2>& region);

/////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Template function declarations (defined in Helpers.hxx) ///////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
template<typename TImage>
void DeepCopy(const typename TImage::Pointer input, typename TImage::Pointer output);

template<typename TPixel>
void GradientFromDerivatives(const typename itk::Image<TPixel, 2>::Pointer xDerivative, const typename itk::Image<TPixel, 2>::Pointer yDerivative, typename itk::Image<itk::CovariantVector<TPixel, 2> >::Pointer output);

template<typename TImage>
void DeepCopyVectorImage(const typename TImage::Pointer input, typename TImage::Pointer output);

template <typename TImage>
void ITKScalarImageToScaledVTKImage(const typename TImage::Pointer image, vtkImageData* outputImage);

template <typename TDebugImageType>
void DebugWriteSequentialImage(const typename TDebugImageType::Pointer image, const std::string& filePrefix, const unsigned int iteration);

template <typename TDebugImageType>
void DebugWriteImageConditional(const typename TDebugImageType::Pointer image, const std::string& fileName, const bool condition);

template <class T>
void WriteScaledScalarImage(const typename T::Pointer image, std::string filename);

template <class T>
void CopyPatch(const typename T::Pointer sourceImage, typename T::Pointer targetImage, const itk::Index<2>& sourcePosition, const itk::Index<2>& targetPosition, const unsigned int radius);

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
void CopySelfPatchIntoValidRegion(typename T::Pointer image, const UnsignedCharScalarImageType::Pointer mask,
                                  const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& destinationRegion);

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
void BlankAndOutlineRegion(typename TImage::Pointer image, const itk::ImageRegion<2>& region, const typename TImage::PixelType& value);

template<typename TImage>
void SetRegionToConstant(typename TImage::Pointer image, const itk::ImageRegion<2>& region,const typename TImage::PixelType& constant);

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
void MaskedBlur(const typename TImage::Pointer inputImage, const Mask::Pointer mask, const unsigned int kernelRadius, typename TImage::Pointer output);

template <typename TImage>
void MaskedDerivative(const typename TImage::Pointer image, const Mask::Pointer mask, const unsigned int direction, FloatScalarImageType::Pointer output);

}// end namespace

#include "Helpers.hxx"

#endif