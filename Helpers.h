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
#include "Types.h"

// ITK
#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
#include "itkNeighborhoodIterator.h"
#include "itkPasteImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// VTK
class vtkImageData;

namespace Helpers
{
  
void RGBImageToCIELabImage(RGBImageType::Pointer rgbImage, FloatVectorImageType::Pointer cielabImage);
  
void NormalizeVectorImage(FloatVector2ImageType::Pointer image);
  
template<typename TImage>
void DeepCopy(typename TImage::Pointer input, typename TImage::Pointer output);

template<typename TImage>
void DeepCopyVectorImage(typename TImage::Pointer input, typename TImage::Pointer output);

void ITKImagetoVTKImage(FloatVectorImageType::Pointer image, vtkImageData* outputImage); // This function simply drives ITKImagetoVTKRGBImage or ITKImagetoVTKMagnitudeImage
void ITKImagetoVTKRGBImage(FloatVectorImageType::Pointer image, vtkImageData* outputImage);
void ITKImagetoVTKMagnitudeImage(FloatVectorImageType::Pointer image, vtkImageData* outputImage);

void ITKImagetoVTKVectorFieldImage(FloatVector2ImageType::Pointer image, vtkImageData* outputImage);

void VectorImageToRGBImage(FloatVectorImageType::Pointer image, RGBImageType::Pointer rgbImage);

template <typename TImage>
void ITKScalarImageToScaledVTKImage(typename TImage::Pointer image, vtkImageData* outputImage);

itk::Index<2> GetRegionCenter(const itk::ImageRegion<2> region);

template <typename TDebugImageType>
void DebugWriteSequentialImage(typename TDebugImageType::Pointer image, const std::string& filePrefix, const unsigned int iteration);

template <typename TDebugImageType>
void DebugWriteImageConditional(typename TDebugImageType::Pointer image, const std::string& fileName, const bool condition);

template <class T>
void WriteScaledScalarImage(typename T::Pointer image, std::string filename);

template <class T>
void CopyPatch(typename T::Pointer sourceImage, typename T::Pointer targetImage, itk::Index<2> sourcePosition, itk::Index<2> targetPosition, unsigned int radius);

template <class T>
void CreateConstantPatch(typename T::Pointer patch, typename T::PixelType value, unsigned int radius);

template<typename T>
void ReplaceValue(typename T::Pointer image, const typename T::PixelType queryValue, const typename T::PixelType replacementValue);

template<typename T>
void WriteImage(typename T::Pointer image, std::string filename);

template <class T>
void CopyPatchIntoImage(typename T::Pointer patch, typename T::Pointer image, itk::Index<2> position);

template <class T>
void CreateBlankPatch(typename T::Pointer patch, const unsigned int radius);

template <class T>
void CopySelfPatchIntoValidRegion(typename T::Pointer image, const UnsignedCharScalarImageType::Pointer mask,
                                  itk::ImageRegion<2> sourceRegion, itk::ImageRegion<2> destinationRegion);

template <class T>
float MaxValue(typename T::Pointer image);

template <class T>
float MaxValueLocation(typename T::Pointer image);

template <class T>
float MinValue(typename T::Pointer image);

template <class T>
itk::Index<2> MinValueLocation(typename T::Pointer image);

template <typename TImage>
void ColorToGrayscale(typename TImage::Pointer colorImage, UnsignedCharScalarImageType::Pointer grayscaleImage);

// Non template function declarations
itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2> pixel, const unsigned int radius);

// template <typename TPixelType>
// float PixelSquaredDifference(const TPixelType&, const TPixelType&);


void BlankAndOutlineImage(vtkImageData*, const unsigned char color[3]);


}// end namespace

#include "Helpers.hxx"

#endif