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

#include "VTKHelpers.h"
#include "Testing.h"

// STL
#include <iostream>
#include <stdexcept>

// VTK
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

static void TestGetCellCenter();

static void TestSetImageCenterPixel();

static void TestBlankAndOutlineImage();

static void TestZeroImage();

static void TestKeepNonZeroVectors();

static void TestMakeValueTransparent();

static void TestMakeImageTransparent();

int main()
{
  TestGetCellCenter();
  TestSetImageCenterPixel();
  TestBlankAndOutlineImage();
  TestZeroImage();
  TestKeepNonZeroVectors();
  TestMakeValueTransparent();
  TestMakeImageTransparent();

  return EXIT_SUCCESS;
}

void TestGetCellCenter()
{
  vtkSmartPointer<vtkImageData> grid = vtkSmartPointer<vtkImageData>::New();
  grid->SetOrigin(0, 0, 0);
  int numVoxelsPerDimension = 5;
  grid->SetSpacing(1, 1, 1);
  int extent[6] = {0, numVoxelsPerDimension, 0, numVoxelsPerDimension, 0, numVoxelsPerDimension};
  grid->SetExtent(extent);
  grid->Update();
  double center[3];
  double correct_center[3] = {.5, .5, .5};

  VTKHelpers::GetCellCenter(grid, 0, center);
  //std::cout << "center: " << center[0] << " " << center[1] << " " << center[2] << std::endl;

  if(!Testing::ValuesEqual(center[0], correct_center[0]) ||
    !Testing::ValuesEqual(center[1], correct_center[1]) ||
    !Testing::ValuesEqual(center[2], correct_center[2]))
    {
    std::stringstream ss;
    ss << "center computed to be: " << center[0] << " " << center[1] << " " << center[2] << std::endl
       << " but is supposed to be: " << correct_center[0] << " " << correct_center[1] << " " << correct_center[2] << std::endl;
    throw std::runtime_error(ss.str());
    }
}

void TestSetImageCenterPixel()
{
  // Set the center pixel of an 'image' to the specified 'color'. The image is assumed to have odd dimensions.
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetNumberOfScalarComponents(3);
  image->SetDimensions(11,11,1);
  image->SetScalarTypeToUnsignedChar();
  image->AllocateScalars();

  unsigned char color[3] = {1,2,3};

  VTKHelpers::SetImageCenterPixel(image, color);

  unsigned char* retrievedColor = static_cast<unsigned char*>(image->GetScalarPointer(5,5,0));
  if(!Testing::ArraysEqual(retrievedColor, color, 3))
    {
    throw std::runtime_error("retrievedColor is " + Testing::ArrayString(retrievedColor, 3) + " and should be " + Testing::ArrayString(color, 3));
    }
}

void TestBlankAndOutlineImage()
{
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetNumberOfScalarComponents(3);
  image->SetDimensions(11,11,1);
  image->SetScalarTypeToUnsignedChar();
  image->AllocateScalars();

  unsigned char black[3] = {0,0,0};
  unsigned char outlineColor[3] = {1,2,3};
  VTKHelpers::BlankAndOutlineImage(image, outlineColor);

  unsigned char* retrievedBoundaryColor = static_cast<unsigned char*>(image->GetScalarPointer(0,0,0));
  if(!Testing::ArraysEqual(retrievedBoundaryColor, outlineColor, 3))
    {
    throw std::runtime_error("retrievedBoundaryColor is " + Testing::ArrayString(retrievedBoundaryColor, 3) + " but should be " + Testing::ArrayString(outlineColor, 3));
    }

  unsigned char* retrievedCenterColor = static_cast<unsigned char*>(image->GetScalarPointer(5,5,0));
  if(!Testing::ArraysEqual(retrievedCenterColor, black, 3))
    {
    throw std::runtime_error("retrievedCenterColor is " + Testing::ArrayString(retrievedCenterColor, 3) + " but should be " + Testing::ArrayString(black, 3));
    }
}

void TestZeroImage()
{
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetScalarTypeToUnsignedChar();
  image->SetDimensions(11,11,1);
  VTKHelpers::ZeroImage(image, 3);

  unsigned char black[3] = {0,0,0};

  unsigned char* retrievedCenterColor = static_cast<unsigned char*>(image->GetScalarPointer(5,5,0));
  if(!Testing::ArraysEqual(retrievedCenterColor, black, 3))
    {
    throw std::runtime_error("retrievedCenterColor is " + Testing::ArrayString(retrievedCenterColor, 3) + " but should be " + Testing::ArrayString(black, 3));
    }
}

void TestKeepNonZeroVectors()
{
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetNumberOfScalarComponents(3);
  image->SetDimensions(11,11,1);
  image->SetScalarTypeToFloat();
  image->AllocateScalars();

  
  
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  VTKHelpers::KeepNonZeroVectors(image, polyData);

}

void TestMakeValueTransparent()
{
  // Test with an image that already has 4 components
  {
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetScalarTypeToUnsignedChar();
  image->SetDimensions(11,11,1);
  VTKHelpers::ZeroImage(image, 4);

  unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(0,0,0));
  pixel[0] = 1;
  pixel[1] = 1;
  pixel[2] = 1;
  unsigned char value[3] = {1,1,1};
  VTKHelpers::MakeValueTransparent(image, value);

  unsigned char* retrievedPixel = static_cast<unsigned char*>(image->GetScalarPointer(0,0,0));
  unsigned char correctPixel[4] = {1,1,1,VTKHelpers::TRANSPARENT};
  if(!Testing::ArraysEqual(retrievedPixel, correctPixel, 4))
    {
    throw std::runtime_error("TestMakeValueTransparent_4: TestMakeValueTransparent: retrievedPixel is " + Testing::ArrayString(retrievedPixel, 4) + " but should be " + Testing::ArrayString(correctPixel, 4));
    }
  }

  // Test with a 3 component image
  {
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetScalarTypeToUnsignedChar();
  image->SetDimensions(11,11,1);
  VTKHelpers::ZeroImage(image, 3);

  unsigned char value[3] = {1,1,1};
  unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(0,0,0));
  pixel[0] = value[0];
  pixel[1] = value[1];
  pixel[2] = value[2];
  
  VTKHelpers::MakeValueTransparent(image, value);

  if(image->GetNumberOfScalarComponents() != 4)
    {
    throw std::runtime_error("image did not expand to 4 channels!");
    }

  unsigned char* retrievedPixel = static_cast<unsigned char*>(image->GetScalarPointer(0,0,0));
  unsigned char correctPixel[4] = {1,1,1,VTKHelpers::TRANSPARENT};
  if(!Testing::ArraysEqual(retrievedPixel, correctPixel, 4))
    {
    throw std::runtime_error("TestMakeValueTransparent_3: retrievedPixel is " + Testing::ArrayString(retrievedPixel, 4) + " but should be " + Testing::ArrayString(correctPixel, 4));
    }
  }

  // Test with a 1 component image
  {
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetScalarTypeToUnsignedChar();
  image->SetDimensions(11,11,1);
  VTKHelpers::ZeroImage(image, 1);
  unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(0,0,0));
  pixel[0] = 1;

  unsigned char value[1] = {1};
  VTKHelpers::MakeValueTransparent(image, value);

  if(image->GetNumberOfScalarComponents() != 4)
    {
    throw std::runtime_error("image did not expand to 4 channels!");
    }

  unsigned char* retrievedPixel = static_cast<unsigned char*>(image->GetScalarPointer(0,0,0));
  unsigned char correctPixel[4] = {1,0,0,VTKHelpers::TRANSPARENT};
  if(!Testing::ArraysEqual(retrievedPixel, correctPixel, 4))
    {
    throw std::runtime_error("TestMakeValueTransparent_1: retrievedPixel is " + Testing::ArrayString(retrievedPixel, 4) + " but should be " + Testing::ArrayString(correctPixel, 4));
    }
  }
}

void TestMakeImageTransparent()
{
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetNumberOfScalarComponents(3);
  image->SetDimensions(11,11,1);
  image->AllocateScalars();
// // Make an entire image transparent.
// void MakeImageTransparent(vtkImageData* image);

}
