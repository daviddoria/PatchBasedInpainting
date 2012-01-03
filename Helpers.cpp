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

#include "Helpers.h"

// VTK
#include <vtkCell.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageMagnitude.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkThresholdPoints.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPolyDataWriter.h>

// Custom
#include "itkRGBToLabColorSpacePixelAccessor.h"
#include "Derivatives.h"

namespace Helpers
{

std::string GetString(const itk::Index<2>& index)
{
  std::stringstream ss;
  ss << "(" << index[0] << ", " << index[1] << ")";
  return ss.str();
}

std::string GetString(const itk::Size<2>& size)
{
  std::stringstream ss;
  ss << "(" << size[0] << ", " << size[1] << ")";
  return ss.str();
}

bool IsValidRGB(const int r, const int g, const int b)
{
  if(r > 255 || r < 0 || g > 255 || g < 0 || b > 255 || b < 0)
  {
    return false;
  }
  return true;
}

std::string GetSequentialFileName(const std::string& filePrefix, const unsigned int iteration, const std::string& fileExtension)
{
  std::stringstream padded;
  padded << filePrefix << "_" << std::setfill('0') << std::setw(4) << iteration << "." << fileExtension;
  return padded.str();
}


float RoundAwayFromZero(const float number)
{
  float signMultiplier = 0;
  if(number >= 0)
    {
    signMultiplier = 1.0;
    }
  else
    {
    signMultiplier = -1.0;
    }
  float absNumber = fabs(number);
  float rounded = ceil(absNumber) * signMultiplier;

  return rounded;
}


unsigned int SideLengthFromRadius(const unsigned int radius)
{
  return radius*2 + 1;
}


void SetImageCenterPixel(vtkImageData* image, const unsigned char color[3])
{
  int dims[3];
  image->GetDimensions(dims);

  unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(dims[0]/2, dims[1]/2, 0));
  pixel[0] = color[0];
  pixel[1] = color[1];
  pixel[2] = color[2];
  pixel[3] = 255; // visible
}

void BlankImage(vtkImageData* image)
{
  image->SetScalarTypeToUnsignedChar();
  image->SetNumberOfScalarComponents(4);
  image->AllocateScalars();

  int dims[3];
  image->GetDimensions(dims);

  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));

      pixel[0] = 0;
      pixel[1] = 0;
      pixel[2] = 0;
      pixel[3] = 0; // transparent
      }
    }
  image->Modified();
}

void BlankAndOutlineImage(vtkImageData* image, const unsigned char color[3])
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
        pixel[3] = 255; // visible
        }
      else
        {
        pixel[0] = 0;
        pixel[1] = 0;
        pixel[2] = 0;
        pixel[3] = 0; // transparent
        }
      }
    }
  image->Modified();
}

void KeepNonZeroVectors(vtkImageData* const image, vtkPolyData* output)
{
  vtkSmartPointer<vtkImageMagnitude> magnitudeFilter = vtkSmartPointer<vtkImageMagnitude>::New();
  magnitudeFilter->SetInputConnection(image->GetProducerPort());
  magnitudeFilter->Update(); // This filter produces a vtkImageData with an array named "Magnitude"

  image->GetPointData()->AddArray(magnitudeFilter->GetOutput()->GetPointData()->GetScalars());
  image->GetPointData()->SetActiveScalars("Magnitude");

  vtkSmartPointer<vtkThresholdPoints> thresholdPoints = vtkSmartPointer<vtkThresholdPoints>::New();
  thresholdPoints->SetInputConnection(image->GetProducerPort());
  thresholdPoints->ThresholdByUpper(.05);
  thresholdPoints->Update();

  output->DeepCopy(thresholdPoints->GetOutput());
  output->GetPointData()->RemoveArray("Magnitude");
  output->GetPointData()->SetActiveScalars("ImageScalars");

  output->Modified();
}



std::string ZeroPad(const unsigned int number, const unsigned int rep)
{
  std::stringstream Padded;
  Padded << std::setfill('0') << std::setw(rep) << number;

  return Padded.str();
}

void MakeImageTransparent(vtkImageData* image)
{
  int dims[3];
  image->GetDimensions(dims);

  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
      pixel[3] = 0; // invisible
      }
    }
  image->Modified();
}

void MakeValueTransparent(vtkImageData* const inputImage, vtkImageData* outputImage, const unsigned char value)
{
  int dims[3];
  inputImage->GetDimensions(dims);

  outputImage->SetScalarTypeToUnsignedChar();
  outputImage->SetNumberOfScalarComponents(4);
  outputImage->SetDimensions(dims);
  outputImage->AllocateScalars();

  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      unsigned char* inputPixel = static_cast<unsigned char*>(inputImage->GetScalarPointer(i,j,0));
      unsigned char* outputPixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(i,j,0));

      /*
      outputPixel[0] = 0;
      outputPixel[1] = inputPixel[0];
      outputPixel[2] = 0;
      */
      outputPixel[0] = inputPixel[0];
      outputPixel[1] = 0;
      outputPixel[2] = 0;

      if(inputPixel[0] == value)
	{
	outputPixel[3] = 0; // invisible
	}
      else
	{
	outputPixel[3] = 255; // visible
	}
      }
    }
}


std::string ReplaceFileExtension(const std::string& fileName, const std::string& newExtension)
{
  // This should be called like:
  // std::string newFilenName = ReplaceFileExtension("oldfile.png", "jpg");
  // To produce "oldfile.jpg"

  std::string newFileName = fileName;
  const unsigned int fileExtensionLength = 3;
  newFileName.replace(newFileName.size() - fileExtensionLength, fileExtensionLength, newExtension);
  return newFileName;
}



void GetCellCenter(vtkImageData* imageData, const unsigned int cellId, double center[3])
{
  double pcoords[3] = {0,0,0};
  std::shared_ptr<double> weights(new double [imageData->GetMaxCellSize()]);
  vtkCell* cell = imageData->GetCell(cellId);
  int subId = cell->GetParametricCenter(pcoords);
  cell->EvaluateLocation(subId, pcoords, center, weights.get());
}

bool StringsMatch(const std::string& a, const std::string& b)
{
  // STL compare returns 0 if strings match. This is unintuitive, so this function returns the expected value.
  if(a.compare(b) == 0)
    {
    return true;
    }
  else
    {
    return false;
    }
}


} // end namespace
