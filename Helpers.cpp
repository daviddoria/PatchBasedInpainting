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

// ITK
#include "itkComposeImageFilter.h"
#include "itkImageAdaptor.h"
#include "itkImageToVectorImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"
#include "itkVectorMagnitudeImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

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

bool IsValidRGB(const int r, const int g, const int b)
{
  if(r > 255 || r < 0 || g > 255 || g < 0 || b > 255 || b < 0)
  {
    return false;
  }
  return true;
}

FloatVector2Type AverageVectors(const std::vector<FloatVector2Type>& vectors)
{
  FloatVector2Type totalVector;
  totalVector.Fill(0);

  if(vectors.size() == 0)
    {
    std::cerr << "Cannot average 0 vectors!" << std::endl;
    return totalVector;
    }

  for(unsigned int i = 0; i < vectors.size(); ++i)
    {
    totalVector[0] += vectors[i][0];
    totalVector[1] += vectors[i][1];
    }

  FloatVector2Type averageVector;
  averageVector[0] = totalVector[0] / static_cast<float>(vectors.size());
  averageVector[1] = totalVector[1] / static_cast<float>(vectors.size());

  return averageVector;
}

std::string GetSequentialFileName(const std::string& filePrefix, const unsigned int iteration, const std::string& fileExtension)
{
  std::stringstream padded;
  padded << filePrefix << "_" << std::setfill('0') << std::setw(4) << iteration << "." << fileExtension;
  return padded.str();
}

void VectorMaskedBlur(const FloatVectorImageType::Pointer inputImage, const Mask::Pointer mask, const float blurVariance, FloatVectorImageType::Pointer output)
{
  // Disassembler
  typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
  IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
  indexSelectionFilter->SetInput(inputImage);

  // Reassembler
  typedef itk::ComposeImageFilter<FloatScalarImageType> ImageToVectorImageFilterType;
  ImageToVectorImageFilterType::Pointer imageToVectorImageFilter = ImageToVectorImageFilterType::New();

  std::vector< FloatScalarImageType::Pointer > filteredImages;

  for(unsigned int i = 0; i < inputImage->GetNumberOfComponentsPerPixel(); ++i)
    {
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->Update();

    FloatScalarImageType::Pointer blurred = FloatScalarImageType::New();
    MaskedBlur<FloatScalarImageType>(indexSelectionFilter->GetOutput(), mask, blurVariance, blurred);

    filteredImages.push_back(blurred);
    imageToVectorImageFilter->SetInput(i, filteredImages[i]);
    }

  imageToVectorImageFilter->Update();

  DeepCopy<FloatVectorImageType>(imageToVectorImageFilter->GetOutput(), output);
}


float AngleBetween(const FloatVector2Type v1, const FloatVector2Type v2)
{
  FloatVector2Type v1normalized = v1;
  v1normalized.Normalize();

  FloatVector2Type v2normalized = v2;
  v2normalized.Normalize();

  return acos(v1normalized*v2normalized);
}

itk::Index<2> GetNextPixelAlongVector(const itk::Index<2>& pixel, const FloatVector2Type& vector)
{
  itk::Index<2> nextPixel = pixel + GetOffsetAlongVector(vector);

  return nextPixel;
}

itk::Offset<2> GetOffsetAlongVector(const FloatVector2Type& vector)
{
  FloatVector2Type normalizedVector = vector;
  normalizedVector.Normalize();

  itk::Offset<2> offset;
  offset[0] = RoundAwayFromZero(normalizedVector[0]);
  offset[1] = RoundAwayFromZero(normalizedVector[1]);

  return offset;
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

std::vector<itk::Offset<2> > Get8NeighborOffsets()
{
  std::vector<itk::Offset<2> > offsets;

  for(int i = -1; i <= 1; ++i)
    {
    for(int j = -1; j <= 1; ++j)
      {
      if(i == 0 && j == 0)
        {
        continue;
        }
      itk::Offset<2> offset;
      offset[0] = i;
      offset[1] = j;
      offsets.push_back(offset);
      }
    }
  return offsets;
}

unsigned int SideLengthFromRadius(const unsigned int radius)
{
  return radius*2 + 1;
}

itk::Size<2> SizeFromRadius(const unsigned int radius)
{
  itk::Size<2> size;
  size.Fill(SideLengthFromRadius(radius));

  return size;
}

void ITKImageToCIELabImage(const FloatVectorImageType* image, FloatVectorImageType* cielabImage)
{
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  VectorImageToRGBImage(image, rgbImage);
  RGBImageToCIELabImage(rgbImage, cielabImage);
}

void RGBImageToCIELabImage(RGBImageType* const rgbImage, FloatVectorImageType* cielabImage)
{
  // The adaptor expects to be able to modify the image (even though we don't in this case),
  // so we cannot pass a const RGBImageType* const.
  // Convert RGB image to Lab color space
  typedef itk::Accessor::RGBToLabColorSpacePixelAccessor<unsigned char, float> RGBToLabColorSpaceAccessorType;
  typedef itk::ImageAdaptor<RGBImageType, RGBToLabColorSpaceAccessorType> RGBToLabAdaptorType;
  RGBToLabAdaptorType::Pointer rgbToLabAdaptor = RGBToLabAdaptorType::New();
  rgbToLabAdaptor->SetImage(rgbImage);

  // Disassembler
  typedef itk::VectorIndexSelectionCastImageFilter<RGBToLabAdaptorType, FloatScalarImageType> VectorIndexSelectionFilterType;
  VectorIndexSelectionFilterType::Pointer vectorIndexSelectionFilter = VectorIndexSelectionFilterType::New();
  vectorIndexSelectionFilter->SetInput(rgbToLabAdaptor);

  std::vector<FloatScalarImageType::Pointer> channels;

  // Reassembler
  typedef itk::ImageToVectorImageFilter<FloatScalarImageType> ReassemblerType;
  typename ReassemblerType::Pointer reassembler = ReassemblerType::New();

  for(unsigned int i = 0; i < 3; ++i)
    {
    channels.push_back(FloatScalarImageType::New());
    vectorIndexSelectionFilter->SetIndex(i);
    vectorIndexSelectionFilter->Update();
    DeepCopy<FloatScalarImageType>(vectorIndexSelectionFilter->GetOutput(), channels[i]);
    reassembler->SetNthInput(i, channels[i]);
    }

  reassembler->Update();

  // Copy to the output
  DeepCopy<FloatVectorImageType>(reassembler->GetOutput(), cielabImage);
}

void VectorImageToRGBImage(const FloatVectorImageType* image, RGBImageType* rgbImage)
{
  // Only the first 3 components are used (assumed to be RGB)
  rgbImage->SetRegions(image->GetLargestPossibleRegion());
  rgbImage->Allocate();

  itk::ImageRegionConstIteratorWithIndex<FloatVectorImageType> inputIterator(image, image->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType inputPixel = inputIterator.Get();
    RGBImageType::PixelType outputPixel;
    outputPixel.SetRed(inputPixel[0]);
    outputPixel.SetGreen(inputPixel[1]);
    outputPixel.SetBlue(inputPixel[2]);

    rgbImage->SetPixel(inputIterator.GetIndex(), outputPixel);
    ++inputIterator;
    }
}

itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2>& pixel, const unsigned int radius)
{
  // This function returns a Region with the specified 'radius' centered at 'pixel'. By the definition of the radius of a square patch, the output region is (radius*2 + 1)x(radius*2 + 1).
  // Note: This region is not necessarily entirely inside the image!

  // The "index" is the lower left corner, so we need to subtract the radius from the center to obtain it
  itk::Index<2> lowerLeft;
  lowerLeft[0] = pixel[0] - radius;
  lowerLeft[1] = pixel[1] - radius;

  itk::ImageRegion<2> region;
  region.SetIndex(lowerLeft);
  itk::Size<2> size;
  size[0] = radius*2 + 1;
  size[1] = radius*2 + 1;
  region.SetSize(size);

  return region;
}


itk::Index<2> GetRegionCenter(const itk::ImageRegion<2>& region)
{
  // This assumes that the region is an odd size in both dimensions
  itk::Index<2> center;
  center[0] = region.GetIndex()[0] + region.GetSize()[0] / 2;
  center[1] = region.GetIndex()[1] + region.GetSize()[1] / 2;

  return center;
}

void NormalizeVectorImage(FloatVector2ImageType* image)
{
  itk::ImageRegionIterator<FloatVector2ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    FloatVector2ImageType::PixelType pixel = imageIterator.Get();
    pixel.Normalize();
    imageIterator.Set(pixel);
    ++imageIterator;
    }
}

void ITKImageToVTKVectorFieldImage(const FloatVector2ImageType* image, vtkImageData* outputImage)
{
  //std::cout << "ITKImagetoVTKVectorFieldImage()" << std::endl;

  // Setup and allocate the image data
  outputImage->SetNumberOfScalarComponents(3); // We really want this to be 2, but VTK complains, so we must add a 3rd component (0) to every pixel
  outputImage->SetScalarTypeToFloat();
  outputImage->SetDimensions(image->GetLargestPossibleRegion().GetSize()[0],
                             image->GetLargestPossibleRegion().GetSize()[1],
                             1);

  outputImage->AllocateScalars();

  // Copy all of the input image pixels to the output image
  itk::ImageRegionConstIteratorWithIndex<FloatVector2ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    float* pixel = static_cast<float*>(outputImage->GetScalarPointer(imageIterator.GetIndex()[0],
								     imageIterator.GetIndex()[1],0));

    FloatVector2ImageType::PixelType inputPixel = imageIterator.Get();
    pixel[0] = inputPixel[0];
    pixel[1] = inputPixel[1];
    pixel[2] = 0;

    ++imageIterator;
    }

  outputImage->GetPointData()->SetActiveVectors("ImageScalars");
  outputImage->Modified();
}

void ITKImageChannelToVTKImage(const FloatVectorImageType* image, const unsigned int channel, vtkImageData* outputImage)
{
  FloatScalarImageType::Pointer channelImage = FloatScalarImageType::New();
  ExtractChannel<float>(image, channel, channelImage);
  ITKScalarImageToScaledVTKImage<FloatScalarImageType>(channelImage, outputImage);
}

// Convert a vector ITK image to a VTK image for display
void ITKVectorImageToVTKImageFromDimension(const FloatVectorImageType* image, vtkImageData* outputImage)
{
  // If the image has 3 channels, assume it is RGB.
  if(image->GetNumberOfComponentsPerPixel() == 3)
    {
    ITKImageToVTKRGBImage(image, outputImage);
    }
  else
    {
    ITKImageToVTKMagnitudeImage(image, outputImage);
    }

  outputImage->Modified();
}

// Convert a vector ITK image to a VTK image for display
void ITKImageToVTKRGBImage(const FloatVectorImageType* image, vtkImageData* outputImage)
{
  // This function assumes an ND (with N>3) image has the first 3 channels as RGB and extra information in the remaining channels.

  //std::cout << "ITKImagetoVTKRGBImage()" << std::endl;
  if(image->GetNumberOfComponentsPerPixel() < 3)
    {
    std::cerr << "The input image has " << image->GetNumberOfComponentsPerPixel() << " components, but at least 3 are required." << std::endl;
    return;
    }

  // Setup and allocate the image data
  outputImage->SetNumberOfScalarComponents(3);
  outputImage->SetScalarTypeToUnsignedChar();
  outputImage->SetDimensions(image->GetLargestPossibleRegion().GetSize()[0],
                             image->GetLargestPossibleRegion().GetSize()[1],
                             1);

  outputImage->AllocateScalars();

  // Copy all of the input image pixels to the output image
  itk::ImageRegionConstIteratorWithIndex<FloatVectorImageType> imageIterator(image,image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    unsigned char* pixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(imageIterator.GetIndex()[0],
                                                                                     imageIterator.GetIndex()[1],0));
    for(unsigned int component = 0; component < 3; component++)
      {
      pixel[component] = static_cast<unsigned char>(imageIterator.Get()[component]);
      }

    ++imageIterator;
    }

  outputImage->Modified();
}


// Convert a vector ITK image to a VTK image for display
void ITKImageToVTKMagnitudeImage(const FloatVectorImageType* image, vtkImageData* outputImage)
{
  //std::cout << "ITKImagetoVTKMagnitudeImage()" << std::endl;
  // Compute the magnitude of the ITK image
  typedef itk::VectorMagnitudeImageFilter<
                  FloatVectorImageType, FloatScalarImageType >  VectorMagnitudeFilterType;

  // Create and setup a magnitude filter
  VectorMagnitudeFilterType::Pointer magnitudeFilter = VectorMagnitudeFilterType::New();
  magnitudeFilter->SetInput( image );
  magnitudeFilter->Update();

  // Rescale and cast for display
  typedef itk::RescaleIntensityImageFilter<
                  FloatScalarImageType, UnsignedCharScalarImageType > RescaleFilterType;

  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->SetInput( magnitudeFilter->GetOutput() );
  rescaleFilter->Update();

  // Setup and allocate the VTK image
  outputImage->SetNumberOfScalarComponents(1);
  outputImage->SetScalarTypeToUnsignedChar();
  outputImage->SetDimensions(image->GetLargestPossibleRegion().GetSize()[0],
                             image->GetLargestPossibleRegion().GetSize()[1],
                             1);

  outputImage->AllocateScalars();

  // Copy all of the scaled magnitudes to the output image
  itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> imageIterator(rescaleFilter->GetOutput(), rescaleFilter->GetOutput()->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    unsigned char* pixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(imageIterator.GetIndex()[0],
                                                                                     imageIterator.GetIndex()[1],0));
    pixel[0] = imageIterator.Get();

    ++imageIterator;
    }

  outputImage->Modified();
}

void SetRegionCenterPixel(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char color[3])
{
  int dims[3];
  image->GetDimensions(dims);

  unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(region.GetIndex()[0] + region.GetSize()[0]/2,
                                                                             region.GetIndex()[1] + region.GetSize()[1]/2, 0));
  pixel[0] = color[0];
  pixel[1] = color[1];
  pixel[2] = color[2];
  pixel[3] = 255; // visible
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

void ConvertNonZeroPixelsToVectors(const FloatVector2ImageType* vectorImage, vtkPolyData* output)
{
  vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
  vectors->SetNumberOfComponents(3);
  vectors->SetName("Vectors");

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  // Copy all of the input image pixels to the output image
  itk::ImageRegionConstIteratorWithIndex<FloatVector2ImageType> imageIterator(vectorImage, vectorImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    FloatVector2ImageType::PixelType inputPixel = imageIterator.Get();
    if(inputPixel.GetNorm() > .05)
      {
      float v[3];
      v[0] = inputPixel[0];
      v[1] = inputPixel[1];
      v[2] = 0;
      vectors->InsertNextTupleValue(v);

      points->InsertNextPoint(imageIterator.GetIndex()[0], imageIterator.GetIndex()[1], 0);
      }

    ++imageIterator;
    }

  output->SetPoints(points);
  output->GetPointData()->SetVectors(vectors);
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

itk::Offset<2> OffsetFrom1DOffset(const itk::Offset<1>& offset1D, const unsigned int dimension)
{
  // Manually construct a 2D offset with 0 in all dimensions except the specified dimension
  itk::Offset<2> offset;
  offset.Fill(0);
  offset[dimension] = offset1D[0];

  return offset;
}

void BlankRegion(vtkImageData* image, const itk::ImageRegion<2>& region)
{
  // Blank the image
  for(unsigned int i = region.GetIndex()[0]; i < region.GetIndex()[0] + region.GetSize()[0]; ++i)
    {
    for(unsigned int j = region.GetIndex()[1]; j < region.GetIndex()[1] + region.GetSize()[1]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i, region.GetIndex()[1], 0));
      pixel[0] = 0;
      pixel[1] = 0;
      pixel[2] = 0;
      pixel[3] = 0; // transparent
      }
    }
  image->Modified();
}

void OutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char color[3])
{
//   std::cout << "Outlining region: " << region << std::endl;
//   std::cout << "Outline color: " << static_cast<int>(value[0]) << " " << static_cast<int>(value[1]) << " " << static_cast<int>(value[2]) << std::endl;
//   std::cout << "Image components: " << image->GetNumberOfScalarComponents() << std::endl;


  // Move along the top and bottom of the region, setting the border pixels.
  for(unsigned int i = region.GetIndex()[0]; i < region.GetIndex()[0] + region.GetSize()[0]; ++i)
    {
    unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i, region.GetIndex()[1], 0));
    pixel[0] = color[0];
    pixel[1] = color[1];
    pixel[2] = color[2];
    pixel[3] = 255; // visible

    pixel = static_cast<unsigned char*>(image->GetScalarPointer(i, region.GetIndex()[1] + region.GetSize()[1] - 1, 0));
    pixel[0] = color[0];
    pixel[1] = color[1];
    pixel[2] = color[2];
    pixel[3] = 255; // visible
    }

  // Move along the left and right of the region, setting the border pixels.
  for(unsigned int j = region.GetIndex()[1]; j < region.GetIndex()[1] + region.GetSize()[1]; ++j)
    {
    unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(region.GetIndex()[0], j, 0));
    pixel[0] = color[0];
    pixel[1] = color[1];
    pixel[2] = color[2];
    pixel[3] = 255; // visible

    pixel = static_cast<unsigned char*>(image->GetScalarPointer(region.GetIndex()[0] + region.GetSize()[0] - 1, j, 0));
    pixel[0] = color[0];
    pixel[1] = color[1];
    pixel[2] = color[2];
    pixel[3] = 255; // visible
    }

  image->Modified();
}

void BlankAndOutlineRegion(vtkImageData* image, const itk::ImageRegion<2>& region, const unsigned char value[3])
{
  BlankRegion(image, region);
  OutlineRegion(image, region, value);
}

// This is a specialization that ensures that the number of pixels per component also matches.
template<>
void DeepCopy<FloatVectorImageType>(const FloatVectorImageType* input, FloatVectorImageType* output)
{
  //std::cout << "DeepCopy<FloatVectorImageType>()" << std::endl;
  bool changed = false;
  if(input->GetNumberOfComponentsPerPixel() != output->GetNumberOfComponentsPerPixel())
    {
    output->SetNumberOfComponentsPerPixel(input->GetNumberOfComponentsPerPixel());
    //std::cout << "Set output NumberOfComponentsPerPixel to " << input->GetNumberOfComponentsPerPixel() << std::endl;
    changed = true;
    }

  if(input->GetLargestPossibleRegion() != output->GetLargestPossibleRegion())
    {
    output->SetRegions(input->GetLargestPossibleRegion());
    changed = true;
    }
  if(changed)
    {
    output->Allocate();
    }

  DeepCopyInRegion<FloatVectorImageType>(input, input->GetLargestPossibleRegion(), output);
}

std::string ReplaceFileExtension(const std::string& fileName, const std::string& newExtension)
{
  // This should be called like:
  // std::string newFilenName = ReplaceFileExtension("oldfile.png", "jpg");
  // To produce "oldfile.jpg"

  std::string newFileName = fileName;
  //newFileName.replace(newFileName.end() - 3, 3, newExtension);
  const unsigned int fileExtensionLength = 3;
  newFileName.replace(newFileName.size() - fileExtensionLength, fileExtensionLength, newExtension);
  return newFileName;
}

itk::ImageRegion<2> CropToRegion(const itk::ImageRegion<2>& inputRegion, const itk::ImageRegion<2>& targetRegion)
{
  // Returns the overlap of the inputRegion with the targetRegion.

  itk::ImageRegion<2> region = targetRegion;
  region.Crop(inputRegion);

  return region;
}

itk::Index<2> FindHighestValueInMaskedRegion(const FloatScalarImageType* image, float& maxValue, UnsignedCharScalarImageType* maskImage)
{
  //EnterFunction("FindHighestValueOnBoundary()");
  // Return the location of the highest pixel in 'image' out of the non-zero pixels in 'boundaryImage'. Return the value of that pixel by reference.
  try
  {
    // Explicity find the maximum on the boundary
    maxValue = 0.0f; // priorities are non-negative, so anything better than 0 will win

    std::vector<itk::Index<2> > boundaryPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(maskImage);

    if(boundaryPixels.size() <= 0)
      {
      std::cerr << "FindHighestValueOnBoundary(): No boundary pixels!" << std::endl;
      exit(-1);
      }

    itk::Index<2> locationOfMaxValue = boundaryPixels[0];

    for(unsigned int i = 0; i < boundaryPixels.size(); ++i)
      {
      if(image->GetPixel(boundaryPixels[i]) > maxValue)
        {
        maxValue = image->GetPixel(boundaryPixels[i]);
        locationOfMaxValue = boundaryPixels[i];
        }
      }
    //DebugMessage<float>("Highest value: ", maxValue);
    //LeaveFunction("FindHighestValueOnBoundary()");
    return locationOfMaxValue;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindHighestValueOnBoundary!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void ComputeColorIsophotesInRegion(const FloatVectorImageType* image, const Mask* mask,
                                   const itk::ImageRegion<2>& region , FloatVector2ImageType* isophotes)
{
  //EnterFunction("ComputeIsophotes()");
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  Helpers::VectorImageToRGBImage(image, rgbImage);

  //HelpersOutput::WriteImageConditional<RGBImageType>(rgbImage, "Debug/Initialize.rgb.mha", this->DebugImages);

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  FloatScalarImageType::Pointer luminanceImage = FloatScalarImageType::New();
  Helpers::DeepCopy<FloatScalarImageType>(luminanceFilter->GetOutput(), luminanceImage);

  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel. From TestIsophotes.cpp, it actually seems like not blurring, but using a masked sobel operator produces the most reliable isophotes.
  unsigned int kernelRadius = 0;
  Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), mask, kernelRadius, blurredLuminance);

  //HelpersOutput::WriteImageConditional<FloatScalarImageType>(blurredLuminance, "Debug/Initialize.blurredLuminance.mha", true);

  Helpers::InitializeImage<FloatVector2ImageType>(isophotes, image->GetLargestPossibleRegion());
  Derivatives::ComputeMaskedIsophotesInRegion(blurredLuminance, mask, region, isophotes);

//   if(this->DebugImages)
//     {
//     HelpersOutput::Write2DVectorImage(this->IsophoteImage, "Debug/Initialize.IsophoteImage.mha");
//     }
  //LeaveFunction("ComputeIsophotes()");
}


void CreatePatchVTKImage(const FloatVectorImageType* image, const itk::ImageRegion<2>& region, vtkImageData* outputImage)
{
  typedef itk::RegionOfInterestImageFilter<FloatVectorImageType,FloatVectorImageType> ExtractFilterType;
  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(region);
  extractFilter->SetInput(image);
  extractFilter->Update();

  Helpers::ITKVectorImageToVTKImageFromDimension(extractFilter->GetOutput(), outputImage);
}

void CreateTransparentVTKImage(const itk::Size<2>& size, vtkImageData* outputImage)
{
  outputImage->SetNumberOfScalarComponents(4);
  outputImage->SetScalarTypeToUnsignedChar();
  outputImage->SetDimensions(size[0], size[1], 1);
  outputImage->AllocateScalars();

  for(unsigned int i = 0; i < size[0]; ++i)
    {
    for(unsigned int j = 0; j < size[0]; ++j)
      {
      unsigned char* pixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(i, j ,0));
      pixel[0] = 0;
      pixel[1] = 0;
      pixel[2] = 0;
      pixel[3] = 0; // transparent
      }
    }
  outputImage->Modified();
}


void GetCellCenter(vtkImageData* imageData, const unsigned int cellId, double center[3])
{
  double pcoords[3] = {0,0,0};
  double *weights = new double [imageData->GetMaxCellSize()];
  vtkCell* cell = imageData->GetCell(cellId);
  int subId = cell->GetParametricCenter(pcoords);
  cell->EvaluateLocation(subId, pcoords, center, weights);
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

void OutputImageType(const itk::ImageBase<2>* input)
{
  if(dynamic_cast<const FloatScalarImageType*>(input))
    {
    std::cout << "Image type FloatScalarImageType" << std::endl;
    }
  else if(dynamic_cast<const Mask*>(input)) // This must come before UnsignedCharScalarImageType
    {
    std::cout << "Image type Mask" << std::endl;
    }
  else if(dynamic_cast<const UnsignedCharScalarImageType*>(input))
    {
    std::cout << "Image type UnsignedCharScalarImageType" << std::endl;
    }
  else if(dynamic_cast<const FloatVectorImageType*>(input))
    {
    std::cout << "Image type FloatVectorImageType" << std::endl;
    }
  else if(dynamic_cast<const FloatVector2ImageType*>(input))
    {
    std::cout << "Image type FloatVector2ImageType" << std::endl;
    }
  else if(dynamic_cast<const IntImageType*>(input))
    {
    std::cout << "Image type IntImageType" << std::endl;
    }
  else
    {
    std::cout << "OutputImageType: Image is Invalid type!" << std::endl;
    }
}

// The return value MUST be a smart pointer
itk::ImageBase<2>::Pointer CreateImageWithSameType(const itk::ImageBase<2>* input)
{
  itk::LightObject::Pointer objectCopyLight = input->CreateAnother();

  itk::ImageBase<2>::Pointer objectCopy = dynamic_cast<itk::ImageBase<2>*>(objectCopyLight.GetPointer());

  return objectCopy;
}

void DeepCopy(const itk::ImageBase<2>* input, itk::ImageBase<2>* output)
{
  if(dynamic_cast<const FloatScalarImageType*>(input))
    {
    std::cout << "Deep copying FloatScalarImageType" << std::endl;
    Helpers::DeepCopy<FloatScalarImageType>(dynamic_cast<const FloatScalarImageType*>(input), dynamic_cast<FloatScalarImageType*>(output));
    }
  else if(dynamic_cast<const Mask*>(input)) // This must come before UnsignedCharScalarImageType because they will both succeed.
    {
    std::cout << "Deep copying Mask" << std::endl;
    Helpers::DeepCopy<Mask>(dynamic_cast<const Mask*>(input), dynamic_cast<Mask*>(output));
    }
  else if(dynamic_cast<const UnsignedCharScalarImageType*>(input))
    {
    std::cout << "Deep copying UnsignedCharScalarImageType" << std::endl;
    Helpers::DeepCopy<UnsignedCharScalarImageType>(dynamic_cast<const UnsignedCharScalarImageType*>(input), dynamic_cast<UnsignedCharScalarImageType*>(output));
    }
  else if(dynamic_cast<const FloatVectorImageType*>(input))
    {
    std::cout << "Deep copying FloatVectorImageType" << std::endl;
    Helpers::DeepCopy<FloatVectorImageType>(dynamic_cast<const FloatVectorImageType*>(input), dynamic_cast<FloatVectorImageType*>(output));
    }
  else if(dynamic_cast<const FloatVector2ImageType*>(input))
    {
    std::cout << "Deep copying FloatVector2ImageType" << std::endl;
    Helpers::DeepCopy<FloatVector2ImageType>(dynamic_cast<const FloatVector2ImageType*>(input), dynamic_cast<FloatVector2ImageType*>(output));
    }
  else if(dynamic_cast<const IntImageType*>(input))
    {
    std::cout << "Deep copying IntImageType" << std::endl;
    Helpers::DeepCopy<IntImageType>(dynamic_cast<const IntImageType*>(input), dynamic_cast<IntImageType*>(output));
    }
  else
    {
    std::cout << "Image is Invalid type!" << std::endl;
    std::cerr << "Cannot cast to any of the specified types!" << std::endl;
    }
}

} // end namespace
