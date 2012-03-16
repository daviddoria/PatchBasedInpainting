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

// STL
#include <memory>
#include <stdexcept>

// VTK
#include <vtkCell.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageMagnitude.h>
#include <vtkImageShiftScale.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkThresholdPoints.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPolyDataWriter.h>

namespace VTKHelpers
{

unsigned char TRANSPARENT = 0;
unsigned char OPAQUE = 255;

void GetCellCenter(vtkImageData* const imageData, const unsigned int cellId, double center[3])
{
  double pcoords[3] = {0,0,0};
  std::shared_ptr<double> weights(new double [imageData->GetMaxCellSize()]);
  vtkCell* cell = imageData->GetCell(cellId);
  int subId = cell->GetParametricCenter(pcoords);
  cell->EvaluateLocation(subId, pcoords, center, weights.get());
}


void SetImageCenterPixel(vtkImageData* const image, const unsigned char color[3])
{
  int dims[3];
  image->GetDimensions(dims);

  int centerPixel[3] = {dims[0]/2, dims[1]/2, 0};
  unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(centerPixel[0], centerPixel[1], centerPixel[2]));
  pixel[0] = color[0];
  pixel[1] = color[1];
  pixel[2] = color[2];
  pixel[3] = 255; // visible
}

void ZeroImage(vtkImageData* const image, const unsigned int channels)
{
  //image->SetScalarTypeToUnsignedChar();
  //image->SetNumberOfScalarComponents(channels);
  //image->AllocateScalars();
  image->AllocateScalars(VTK_UNSIGNED_CHAR, channels);

  int dims[3];
  image->GetDimensions(dims);

  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
      for(unsigned int channel = 0; channel < channels; ++channel)
        {
        pixel[channel] = 0;
        }
      }
    }
  image->Modified();
}

void BlankAndOutlineImage(vtkImageData* const image, const unsigned char color[3])
{
  int dims[3];
  image->GetDimensions(dims);

  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
      if(i == 0 || i == dims[0] - 1 || j == 0 || j == dims[1] - 1)
        {
        pixel[0] = color[0];
        pixel[1] = color[1];
        pixel[2] = color[2];
        pixel[3] = OPAQUE;
        }
      else
        {
        pixel[0] = 0;
        pixel[1] = 0;
        pixel[2] = 0;
        pixel[3] = TRANSPARENT;
        }
      }
    }
  image->Modified();
}

void KeepNonZeroVectors(vtkImageData* const image, vtkPolyData* output)
{
  vtkSmartPointer<vtkImageMagnitude> magnitudeFilter = vtkSmartPointer<vtkImageMagnitude>::New();
  magnitudeFilter->SetInputData(image);
  magnitudeFilter->Update(); // This filter produces a vtkImageData with an array named "Magnitude"

  image->GetPointData()->AddArray(magnitudeFilter->GetOutput()->GetPointData()->GetScalars());
  image->GetPointData()->SetActiveScalars("Magnitude");

  vtkSmartPointer<vtkThresholdPoints> thresholdPoints = vtkSmartPointer<vtkThresholdPoints>::New();
  thresholdPoints->SetInputData(image);
  thresholdPoints->ThresholdByUpper(.05);
  thresholdPoints->Update();

  output->DeepCopy(thresholdPoints->GetOutput());
  output->GetPointData()->RemoveArray("Magnitude");
  output->GetPointData()->SetActiveScalars("ImageScalars");

  output->Modified();
}


void MakeImageTransparent(vtkImageData* const image)
{
  int dims[3];
  image->GetDimensions(dims);

  if(image->GetNumberOfScalarComponents() < 4)
    {
    //image->SetNumberOfScalarComponents(4);
    //image->AllocateScalars();
    image->AllocateScalars(image->GetScalarType(), 4);
    }

  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
      pixel[3] = TRANSPARENT;
      }
    }
  image->Modified();
}

void MakeValueTransparent(vtkImageData* const image, const unsigned char value[3])
{
  if(image->GetScalarType() != VTK_UNSIGNED_CHAR)
    {
    throw std::runtime_error("Image must be unsigned char to set values to transparent!");
    }

  vtkSmartPointer<vtkImageData> originalImage = vtkSmartPointer<vtkImageData>::New();
  originalImage->DeepCopy(image);

  int dims[3];
  image->GetDimensions(dims);

  unsigned int originalNumberOfComponents = image->GetNumberOfScalarComponents();
  if(originalNumberOfComponents < 4)
    {
    //image->SetNumberOfScalarComponents(4);
    //image->AllocateScalars();
    image->AllocateScalars(image->GetScalarType(), 4);
    }

  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
      unsigned char* oldPixel = static_cast<unsigned char*>(originalImage->GetScalarPointer(i,j,0));

      // Fill all old channels with original values
      for(unsigned int channel = 0; channel < originalNumberOfComponents; ++channel)
        {
        pixel[channel] = oldPixel[channel];
        }

      // Fill all new channels with 0
      for(unsigned int channel = originalNumberOfComponents; channel < 3; ++channel)
        {
        pixel[channel] = 0;
        }

      bool setTransparent = false;
      for(unsigned int channel = 0; channel < originalNumberOfComponents; ++channel)
        {
        if(pixel[channel] == value[channel])
          {
          setTransparent = true;
          }
        }

      if(setTransparent)
        {
        pixel[3] = TRANSPARENT;
        }
      else
        {
        pixel[3] = OPAQUE;
        }
      } // end for j
    } // end for i
}

void OutputAllArrayNames(vtkPolyData* const polyData)
{
  std::cout << "Arrays in polyData (there are " << polyData->GetPointData()->GetNumberOfArrays() << ") : " << std::endl;
  for(vtkIdType arrayId = 0; arrayId < polyData->GetPointData()->GetNumberOfArrays(); arrayId++)
    {
    std::cout << polyData->GetPointData()->GetArrayName(arrayId) << std::endl;
    }
}

void ScaleImage(vtkImageData* const image)
{
  double valuesRange[2];
  
//   vtkDoubleArray* values = vtkDoubleArray::SafeDownCast(image->GetPointData()->GetArray("ImageScalars"));
//   if(values)
//   {
//     values->GetValueRange(valuesRange);
//   }
//   else
//   {
//     throw std::runtime_error("Could not get ImageScalars array!");
//   }

  valuesRange[0] = image->GetScalarRange()[0];
  valuesRange[1] = image->GetScalarRange()[1];
  
  std::cout << "valuesRange = " << valuesRange[0] << " " << valuesRange[1] << std::endl;
 
  vtkSmartPointer<vtkImageShiftScale> shiftScaleFilter =
    vtkSmartPointer<vtkImageShiftScale>::New();
  shiftScaleFilter->SetOutputScalarTypeToUnsignedChar();
  shiftScaleFilter->SetInputData(image);
  shiftScaleFilter->SetShift(-1.0 * valuesRange[0]);
  shiftScaleFilter->SetScale(255.0f * (valuesRange[1] - valuesRange[0]));
  shiftScaleFilter->Update();

  image->DeepCopy(shiftScaleFilter->GetOutput());
}

} // end namespace
