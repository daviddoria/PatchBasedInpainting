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
#include "itkImageAdaptor.h"
#include "itkImageToVectorImageFilter.h"
#include "itkVectorMagnitudeImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

// VTK
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageMagnitude.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkThresholdPoints.h>

// Custom
#include "itkRGBToLabColorSpacePixelAccessor.h"

namespace Helpers
{

bool HasHoleNeighbor(const itk::Index<2>& pixel, const Mask::Pointer mask)
{
  itk::Offset<2> offset;
  offset[0] = 0;
  offset[1] = 1;
  
  if(mask->IsHole(pixel + offset))
    {
    return true;
    }

  offset[0] = 1;
  offset[1] = 0;
  if(mask->IsHole(pixel + offset))
    {
    return true;
    }

  offset[0] = 0;
  offset[1] = -1;
  if(mask->IsHole(pixel + offset))
    {
    return true;
    }

  offset[0] = -1;
  offset[1] = 0;
  if(mask->IsHole(pixel + offset))
    {
    return true;
    }

  offset[0] = 1;
  offset[1] = 1;
  if(mask->IsHole(pixel + offset))
    {
    return true;
    }

  offset[0] = -1;
  offset[1] = -1;
  if(mask->IsHole(pixel + offset))
    {
    return true;
    }

  offset[0] = 1;
  offset[1] = -1;
  if(mask->IsHole(pixel + offset))
    {
    return true;
    }

  offset[0] = -1;
  offset[1] = 1;
  if(mask->IsHole(pixel + offset))
    {
    return true;
    }

  return false;
}

unsigned int SideLengthFromRadius(const unsigned int radius)
{
  return radius*2 + 1;
}

void RGBImageToCIELabImage(const RGBImageType::Pointer rgbImage, FloatVectorImageType::Pointer cielabImage)
{
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
  DeepCopyVectorImage<FloatVectorImageType>(reassembler->GetOutput(), cielabImage);
}

void VectorImageToRGBImage(const FloatVectorImageType::Pointer image, RGBImageType::Pointer rgbImage)
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


// Convert a vector ITK image to a VTK image for display
void ITKVectorImagetoVTKImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage)
{
  //std::cout << "ITKImagetoVTKImage()" << std::endl;
  if(image->GetNumberOfComponentsPerPixel() >= 3)
    {
    ITKImagetoVTKRGBImage(image, outputImage);
    }
  else
    {
    ITKImagetoVTKMagnitudeImage(image, outputImage);
    }
    
  outputImage->Modified();
}

void NormalizeVectorImage(FloatVector2ImageType::Pointer image)
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

void ITKImagetoVTKVectorFieldImage(const FloatVector2ImageType::Pointer image, vtkImageData* outputImage)
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

// Convert a vector ITK image to a VTK image for display
void ITKImagetoVTKRGBImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage)
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
void ITKImagetoVTKMagnitudeImage(const FloatVectorImageType::Pointer image, vtkImageData* outputImage)
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


void BlankAndOutlineImage(vtkImageData* image, const unsigned char color[3])
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

void KeepNonZeroVectors(vtkImageData* image, vtkPolyData* output)
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

void ConvertNonZeroPixelsToVectors(const FloatVector2ImageType::Pointer vectorImage, vtkPolyData* output)
{
  vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
  vectors->SetNumberOfComponents(3);
  
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

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegion(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region)
{
  
  std::vector<HistogramType::Pointer> histograms;
  
  typedef itk::RegionOfInterestImageFilter< FloatVectorImageType, FloatVectorImageType > RegionOfInterestImageFilterType;
  RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  // Compute the histogram of each channel
  for(unsigned int i = 0; i < image->GetNumberOfComponentsPerPixel(); ++i)
    {
    typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->SetInput(image);
    indexSelectionFilter->GetOutput()->SetRequestedRegion(region);
    indexSelectionFilter->Update();
  
    const unsigned int MeasurementVectorSize = 1;
    const unsigned int binsPerDimension = 30;
    
    HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
    lowerBound.Fill(0);
    
    HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
    upperBound.Fill(255) ;
    
    HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);
    
    ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
    imageToHistogramFilter->SetInput(indexSelectionFilter->GetOutput());
    imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
    imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
    imageToHistogramFilter->SetHistogramSize(size);
    imageToHistogramFilter->SetAutoMinimumMaximum(false);
    imageToHistogramFilter->Update();

    histograms.push_back(imageToHistogramFilter->GetOutput());
    }
    
  return histograms;
}


std::vector<HistogramType::Pointer> ComputeHistogramsOfMaskedRegion(const FloatVectorImageType::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region)
{
  
  std::vector<HistogramType::Pointer> histograms;

  // Compute the histogram of each channel
  for(unsigned int i = 0; i < image->GetNumberOfComponentsPerPixel(); ++i)
    {
      
    typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->SetInput(image);
    indexSelectionFilter->GetOutput()->SetRequestedRegion(region);
    indexSelectionFilter->Update();
  
    const unsigned int MeasurementVectorSize = 1;
    const unsigned int binsPerDimension = 30;
    
    HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
    lowerBound.Fill(0);
    
    HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
    upperBound.Fill(255) ;
    
    HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);
    
    
    itk::ImageRegionConstIterator<FloatScalarImageType> imageIterator(indexSelectionFilter->GetOutput(), region);
    
    HistogramType::Pointer histogram = HistogramType::New();

    histogram->SetMeasurementVectorSize(MeasurementVectorSize);
 
    histogram->Initialize(size, lowerBound, upperBound );
  
    while(!imageIterator.IsAtEnd())
      {
      if(mask->IsHole(imageIterator.GetIndex()))
	{
	++imageIterator;
	continue;
	}
      float pixel = imageIterator.Get();
      HistogramType::MeasurementVectorType mv(MeasurementVectorSize);
      mv[0] = pixel;

      histogram->IncreaseFrequencyOfMeasurement(mv, 1);
      
      ++imageIterator;
      }
      
    histograms.push_back(histogram);
    }
    
  return histograms;
}

std::vector<HistogramType::Pointer> ComputeHistogramsOfRegionManual(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region)
{
  
  std::vector<HistogramType::Pointer> histograms;

  // Compute the histogram of each channel
  for(unsigned int i = 0; i < image->GetNumberOfComponentsPerPixel(); ++i)
    {
      
    typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, FloatScalarImageType> IndexSelectionType;
    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->SetInput(image);
    indexSelectionFilter->GetOutput()->SetRequestedRegion(region);
    indexSelectionFilter->Update();
  
    const unsigned int MeasurementVectorSize = 1;
    const unsigned int binsPerDimension = 30;
    
    HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
    lowerBound.Fill(0);
    
    HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
    upperBound.Fill(255) ;
    
    HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);
    
    
    itk::ImageRegionConstIterator<FloatScalarImageType> imageIterator(indexSelectionFilter->GetOutput(), region);
    
    HistogramType::Pointer histogram = HistogramType::New();

    histogram->SetMeasurementVectorSize(MeasurementVectorSize);
 
    histogram->Initialize(size, lowerBound, upperBound );
  
    while(!imageIterator.IsAtEnd())
      {

      float pixel = imageIterator.Get();
      HistogramType::MeasurementVectorType mv(MeasurementVectorSize);
      mv[0] = pixel;

      histogram->IncreaseFrequencyOfMeasurement(mv, 1);
      
      ++imageIterator;
      }
      
    histograms.push_back(histogram);
    }
    
  return histograms;
}


HistogramType::Pointer ComputeNDHistogramOfRegionManual(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension)
{
  const unsigned int MeasurementVectorSize = image->GetNumberOfComponentsPerPixel();
  
  HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
  lowerBound.Fill(0);

  HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
  upperBound.Fill(255) ;

  HistogramType::SizeType size(MeasurementVectorSize);
  size.Fill(binsPerDimension);

  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, region);

  HistogramType::Pointer histogram = HistogramType::New();

  histogram->SetMeasurementVectorSize(MeasurementVectorSize);

  histogram->Initialize(size, lowerBound, upperBound );

  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    HistogramType::MeasurementVectorType mv(MeasurementVectorSize);
    for(unsigned int i = 0; i < MeasurementVectorSize; ++i)
      {
      mv[i] = pixel[i];
      }

    histogram->IncreaseFrequencyOfMeasurement(mv, 1);

    ++imageIterator;
    }

  return histogram;
}


HistogramType::Pointer ComputeNDHistogramOfMaskedRegionManual(const FloatVectorImageType::Pointer image, const Mask::Pointer mask, const itk::ImageRegion<2>& region, const unsigned int binsPerDimension)
{
  const unsigned int MeasurementVectorSize = image->GetNumberOfComponentsPerPixel();

  HistogramType::MeasurementVectorType lowerBound(MeasurementVectorSize);
  lowerBound.Fill(0);

  HistogramType::MeasurementVectorType upperBound(MeasurementVectorSize);
  upperBound.Fill(255) ;

  HistogramType::SizeType size(MeasurementVectorSize);
  size.Fill(binsPerDimension);

  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, region);

  HistogramType::Pointer histogram = HistogramType::New();

  histogram->SetMeasurementVectorSize(MeasurementVectorSize);

  histogram->Initialize(size, lowerBound, upperBound );

  while(!imageIterator.IsAtEnd())
    {
    if(mask->IsHole(imageIterator.GetIndex()))
      {
      ++imageIterator;
      continue;
      }
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    HistogramType::MeasurementVectorType mv(MeasurementVectorSize);
    for(unsigned int i = 0; i < MeasurementVectorSize; ++i)
      {
      mv[i] = pixel[i];
      }

    histogram->IncreaseFrequencyOfMeasurement(mv, 1);

    ++imageIterator;
    }

  return histogram;
}

void OutputHistogram(const HistogramType::Pointer histogram)
{
  for(unsigned int i = 0; i < histogram->GetSize(0); ++i)
    {
    std::cout << histogram->GetFrequency(i) << " ";
    }
  std::cout << std::endl;
}

float NDHistogramDifference(const HistogramType::Pointer histogram1, const HistogramType::Pointer histogram2)
{
  unsigned int totalBins = 1;
  for(unsigned int i = 0; i < histogram1->GetSize().GetNumberOfElements(); ++i)
    {
    totalBins *= histogram1->GetSize(i);
    if(histogram1->GetSize(i) != histogram2->GetSize(i))
      {
      std::cerr << "Histograms must be the same size!" << std::endl;
      return 0;
      }
    }

  float totalDifference = 0;
  
  for(unsigned int i = 0; i < totalBins; ++i)
    {
    // The casts to float are necessary other wise the integer division always ends up = 0 !
    float normalized1 = static_cast<float>(histogram1->GetFrequency(i))/static_cast<float>(histogram1->GetTotalFrequency());
    float normalized2 = static_cast<float>(histogram2->GetFrequency(i))/static_cast<float>(histogram2->GetTotalFrequency());
    float difference = fabs(normalized1 - normalized2);
    //std::cout << "difference: " << difference << std::endl;
    totalDifference += difference;
    }

  //std::cout << "totalDifference: " << totalDifference << std::endl;
  return totalDifference;
}

float HistogramDifference(const HistogramType::Pointer histogram1, const HistogramType::Pointer histogram2)
{
  if(histogram1->GetSize(0) != histogram2->GetSize(0))
    {
    std::cerr << "Histograms must be the same size!" << std::endl;
    return 0;
    }

  float totalDifference = 0;
  for(unsigned int i = 0; i < histogram1->GetSize(0); ++i)
    {
//     std::cout << "h1 " << i << " " << histogram1->GetFrequency(i) << std::endl;
//     std::cout << "h1 total " << histogram1->GetTotalFrequency() << std::endl;
// 
//     std::cout << "h2 " << i << " " << histogram2->GetFrequency(i) << std::endl;
//     std::cout << "h2 total " << histogram2->GetTotalFrequency() << std::endl;

    // The casts to float are necessary other wise the integer division always ends up = 0 !
    float normalized1 = static_cast<float>(histogram1->GetFrequency(i))/static_cast<float>(histogram1->GetTotalFrequency());
    float normalized2 = static_cast<float>(histogram2->GetFrequency(i))/static_cast<float>(histogram2->GetTotalFrequency());
    //float difference = fabs(normalized1 - normalized2);
    float difference = (normalized1 - normalized2)*(normalized1 - normalized2);
    //std::cout << "difference: " << difference << std::endl;
    totalDifference += difference;
    }

  //std::cout << "totalDifference: " << totalDifference << std::endl;
  return totalDifference;
}

std::string ZeroPad(const unsigned int number, const unsigned int rep)
{
  std::stringstream Padded;
  Padded << std::setfill('0') << std::setw(rep) << number;

  return Padded.str();
}

void MakePixelsTransparent(vtkImageData* inputImage, vtkImageData* outputImage, const unsigned char value)
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

      outputPixel[0] = 0;
      outputPixel[1] = inputPixel[0];
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


QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx)
{
  // The fudge factors so that the scroll bars do not appear
  
  unsigned int fudge = 6;
  if(gfx->height() < gfx->width())
    {
    return qimage.scaledToHeight(gfx->height() - fudge);
    }
  else
    {
    return qimage.scaledToWidth(gfx->width() - fudge);
    }
}

} // end namespace
