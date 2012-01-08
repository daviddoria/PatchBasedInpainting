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

#ifndef ITKVTKHELPERS_H
#define ITKVTKHELPERS_H

#include "itkSize.h"

// Custom
#include "Types.h"

// VTK
class vtkImageData;
class vtkPolyData;

// ITK
#include "itkImageRegion.h"

namespace ITKVTKHelpers
{
  
void CreateTransparentVTKImage(const itk::Size<2>& size, vtkImageData* const outputImage);


// Set the center pixel of a 'region' in an 'image' to the specified 'color'. The region is assumed to have odd dimensions.
void SetRegionCenterPixel(vtkImageData* const image, const itk::ImageRegion<2>& region, const unsigned char color[3]);

// This function simply drives ITKImagetoVTKRGBImage or ITKImagetoVTKMagnitudeImage based on the number of components of the input.
void ITKVectorImageToVTKImageFromDimension(const FloatVectorImageType* const image, vtkImageData* const outputImage);

// These functions create a VTK image from a multidimensional ITK image.
void ITKImageToVTKRGBImage(const FloatVectorImageType* const image, vtkImageData* const outputImage);
void ITKImageToVTKMagnitudeImage(const FloatVectorImageType* const image, vtkImageData* const outputImage);
void ITKImageChannelToVTKImage(const FloatVectorImageType* const image, const unsigned int channel, vtkImageData* const outputImage);


// Create a VTK image of a patch of an image.
void CreatePatchVTKImage(const FloatVectorImageType* image, const itk::ImageRegion<2>& region, vtkImageData* outputImage);

// Create a VTK image filled with values representing vectors. (There is no concept of a "vector image" in VTK).
void ITKImageToVTKVectorFieldImage(const FloatVector2ImageType* image, vtkImageData* outputImage);


// It is too intensive to glyph every vector in a vector image. In many cases, the vector field may have
// very large regions of zero vectors. This function creates the vectors for only the non-zero pixels in
// the vector image.
void ConvertNonZeroPixelsToVectors(const FloatVector2ImageType* const vectorImage, vtkPolyData* const output);


// Simply calls OutlineRegion followed by BlankRegion
void BlankAndOutlineRegion(vtkImageData* const image, const itk::ImageRegion<2>& region, const unsigned char value[3]);

// Set pixels on the boundary of 'region' in 'image' to 'value'.
void OutlineRegion(vtkImageData* const image, const itk::ImageRegion<2>& region, const unsigned char value[3]);

// Set all pixels in 'region' in 'image' to black.
void BlankRegion(vtkImageData* const image, const itk::ImageRegion<2>& region);


template <typename TImage>
void ITKScalarImageToScaledVTKImage(const TImage* const image, vtkImageData* const outputImage);


} // end namespace

#include "ITKVTKHelpers.hxx"

#endif
