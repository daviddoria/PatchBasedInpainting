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

#include "ITKVTKHelpers.h"

// STL
#include <iostream>

// VTK
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

static void TestCreateTransparentVTKImage();
static void TestSetRegionCenterPixel();
static void TestITKVectorImageToVTKImageFromDimension();
static void TestITKImageToVTKRGBImage();
static void TestITKImageToVTKMagnitudeImage();
static void TestITKImageChannelToVTKImage();
static void TestCreatePatchVTKImage();
static void TestITKImageToVTKVectorFieldImage();
static void TestConvertNonZeroPixelsToVectors();
static void TestBlankAndOutlineRegion();
static void TestOutlineRegion();
static void TestBlankRegion();
static void TestITKScalarImageToScaledVTKImage();

int main()
{
  TestCreateTransparentVTKImage();
  TestSetRegionCenterPixel();
  TestITKVectorImageToVTKImageFromDimension();
  TestITKImageToVTKRGBImage();
  TestITKImageToVTKMagnitudeImage();
  TestITKImageChannelToVTKImage();
  TestCreatePatchVTKImage();
  TestITKImageToVTKVectorFieldImage();
  TestConvertNonZeroPixelsToVectors();
  TestBlankAndOutlineRegion();
  TestOutlineRegion();
  TestBlankRegion();
  TestITKScalarImageToScaledVTKImage();

  return EXIT_SUCCESS;
}

void TestCreateTransparentVTKImage()
{
  itk::Size<2> size = {{10,10}};
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  ITKVTKHelpers::CreateTransparentVTKImage(size, image);
  unsigned char correctPixel[4] = {0,0,0,0}; // The last zero means 'transparent'
  for(unsigned int i = 0; i < size[0]; ++i)
    {
    for(unsigned int j = 0; j < size[1]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i, j ,0));
      for(unsigned int component = 0; component < 4; ++component)
        {
        if(pixel[component] != correctPixel[component])
          {
          std::stringstream ss;
          ss << "Pixel (" << i << ", " << j << ") is (" << pixel[0] << ", " << pixel[1] << ", " << pixel[2] << ", " << pixel[3] << ") "
             << " but should be (" << correctPixel[0] << ", " << correctPixel[1] << ", " << correctPixel[2] << ", " << correctPixel[3] << ") ";
          throw std::runtime_error(ss.str());
          }
        }
      }
    }
}

void TestSetRegionCenterPixel()
{
// // Set the center pixel of a 'region' in an 'image' to the specified 'color'. The region is assumed to have odd dimensions.
// void SetRegionCenterPixel(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char color[3]);
  throw;
}

void TestITKVectorImageToVTKImageFromDimension()
{
// // This function simply drives ITKImagetoVTKRGBImage or ITKImagetoVTKMagnitudeImage based on the number of components of the input.
// void ITKVectorImageToVTKImageFromDimension(const FloatVectorImageType* image, vtkImageData* outputImage);
  throw;
}

void TestITKImageToVTKRGBImage()
{
// // These functions create a VTK image from a multidimensional ITK image.
// void ITKImageToVTKRGBImage(const FloatVectorImageType* image, vtkImageData* outputImage);
  throw;
}

void TestITKImageToVTKMagnitudeImage()
{
// void ITKImageToVTKMagnitudeImage(const FloatVectorImageType* image, vtkImageData* outputImage);
  throw;
}

void TestITKImageChannelToVTKImage()
{
// void ITKImageChannelToVTKImage(const FloatVectorImageType* image, const unsigned int channel, vtkImageData* outputImage);
  throw;
}

void TestCreatePatchVTKImage()
{
//
// // Create a VTK image of a patch of an image.
// void CreatePatchVTKImage(const FloatVectorImageType* image, const itk::ImageRegion<2>& region, vtkImageData* outputImage);
  throw;
}

void TestITKImageToVTKVectorFieldImage()
{
//
// // Create a VTK image filled with values representing vectors. (There is no concept of a "vector image" in VTK).
// void ITKImageToVTKVectorFieldImage(const FloatVector2ImageType* image, vtkImageData* outputImage);
  throw;
}

void TestConvertNonZeroPixelsToVectors()
{
//
//
// // It is too intensive to glyph every vector in a vector image. In many cases, the vector field may have
// // very large regions of zero vectors. This function creates the vectors for only the non-zero pixels in
// // the vector image.
// void ConvertNonZeroPixelsToVectors(const FloatVector2ImageType* vectorImage, vtkPolyData* output);
  throw;
}

void TestBlankAndOutlineRegion()
{
//
//
// // Simply calls OutlineRegion followed by BlankRegion
// void BlankAndOutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3]);
  throw;
}

void TestOutlineRegion()
{
//
// // Set pixels on the boundary of 'region' in 'image' to 'value'.
// void OutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3]);
  throw;
}

void TestBlankRegion()
{
//
// // Set all pixels in 'region' in 'image' to black.
// void BlankRegion(vtkImageData* image, const itk::ImageRegion<2>& region);
  throw;
}

void TestITKScalarImageToScaledVTKImage()
{
//
//
// template <typename TImage>
// void ITKScalarImageToScaledVTKImage(const TImage* image, vtkImageData* outputImage);
  throw;
}

