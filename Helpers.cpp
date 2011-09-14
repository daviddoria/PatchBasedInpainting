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
#include <vtkImageData.h>
#include <vtkPointData.h>

// Custom
#include "itkRGBToLabColorSpacePixelAccessor.h"

namespace Helpers
{

void RGBImageToCIELabImage(RGBImageType::Pointer rgbImage, FloatVectorImageType::Pointer cielabImage)
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

void VectorImageToRGBImage(FloatVectorImageType::Pointer image, RGBImageType::Pointer rgbImage)
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

itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2> pixel, const unsigned int radius)
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


itk::Index<2> GetRegionCenter(const itk::ImageRegion<2> region)
{
  itk::Index<2> center;
  center[0] = region.GetIndex()[0] + region.GetSize()[0] / 2;
  center[1] = region.GetIndex()[1] + region.GetSize()[1] / 2;

  return center;
}


// Convert a vector ITK image to a VTK image for display
void ITKImagetoVTKImage(FloatVectorImageType::Pointer image, vtkImageData* outputImage)
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

void ITKImagetoVTKVectorFieldImage(FloatVector2ImageType::Pointer image, vtkImageData* outputImage)
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
void ITKImagetoVTKRGBImage(FloatVectorImageType::Pointer image, vtkImageData* outputImage)
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
void ITKImagetoVTKMagnitudeImage(FloatVectorImageType::Pointer image, vtkImageData* outputImage)
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

} // end namespace