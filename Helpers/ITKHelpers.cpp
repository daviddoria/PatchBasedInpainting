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

#include "ITKHelpers.h"

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
#include "ImageProcessing/Derivatives.h"
#include "Helpers.h"
#include "ImageProcessing/itkRGBToLabColorSpacePixelAccessor.h"

namespace ITKHelpers
{

std::string GetIndexString(const itk::Index<2>& index)
{
  std::stringstream ss;
  ss << "(" << index[0] << ", " << index[1] << ")";
  return ss.str();
}

std::string GetSizeString(const itk::Size<2>& size)
{
  std::stringstream ss;
  ss << "(" << size[0] << ", " << size[1] << ")";
  return ss.str();
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

float AngleBetween(const FloatVector2Type& v1, const FloatVector2Type& v2)
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
  offset[0] = Helpers::RoundAwayFromZero(normalizedVector[0]);
  offset[1] = Helpers::RoundAwayFromZero(normalizedVector[1]);

  return offset;
}


itk::Size<2> SizeFromRadius(const unsigned int radius)
{
  itk::Size<2> size;
  size.Fill(Helpers::SideLengthFromRadius(radius));

  return size;
}

void ITKImageToCIELabImage(const FloatVectorImageType* const image, FloatVectorImageType* const cielabImage)
{
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  VectorImageToRGBImage(image, rgbImage);
  RGBImageToCIELabImage(rgbImage, cielabImage);
}

void RGBImageToCIELabImage(RGBImageType* const rgbImage, FloatVectorImageType* const cielabImage)
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

void VectorImageToRGBImage(const FloatVectorImageType* const image, RGBImageType* const rgbImage)
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

itk::Offset<2> OffsetFrom1DOffset(const itk::Offset<1>& offset1D, const unsigned int dimension)
{
  // Manually construct a 2D offset with 0 in all dimensions except the specified dimension
  itk::Offset<2> offset;
  offset.Fill(0);
  offset[dimension] = offset1D[0];

  return offset;
}


// This is a specialization that ensures that the number of pixels per component also matches.
template<>
void DeepCopy<FloatVectorImageType>(const FloatVectorImageType* const input, FloatVectorImageType* const output)
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


itk::ImageRegion<2> CropToRegion(const itk::ImageRegion<2>& inputRegion, const itk::ImageRegion<2>& targetRegion)
{
  // Returns the overlap of the inputRegion with the targetRegion.

  itk::ImageRegion<2> region = targetRegion;
  region.Crop(inputRegion);

  return region;
}

void OutputImageType(const itk::ImageBase<2>* const input)
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
itk::ImageBase<2>::Pointer CreateImageWithSameType(const itk::ImageBase<2>* const input)
{
  itk::LightObject::Pointer objectCopyLight = input->CreateAnother();

  itk::ImageBase<2>::Pointer objectCopy = dynamic_cast<itk::ImageBase<2>*>(objectCopyLight.GetPointer());

  return objectCopy;
}
/*
void DeepCopyUnknownType(const itk::ImageBase<2>* const input, itk::ImageBase<2>* const output)
{
  if(dynamic_cast<const FloatScalarImageType*>(input))
    {
    std::cout << "Deep copying FloatScalarImageType" << std::endl;
    ITKHelpers::DeepCopy<FloatScalarImageType>(dynamic_cast<const FloatScalarImageType*>(input), dynamic_cast<FloatScalarImageType*>(output));
    }
  else if(dynamic_cast<const Mask*>(input)) // This must come before UnsignedCharScalarImageType because they will both succeed.
    {
    std::cout << "Deep copying Mask" << std::endl;
    ITKHelpers::DeepCopy<Mask>(dynamic_cast<const Mask*>(input), dynamic_cast<Mask*>(output));
    }
  else if(dynamic_cast<const UnsignedCharScalarImageType*>(input))
    {
    std::cout << "Deep copying UnsignedCharScalarImageType" << std::endl;
    ITKHelpers::DeepCopy<UnsignedCharScalarImageType>(dynamic_cast<const UnsignedCharScalarImageType*>(input), dynamic_cast<UnsignedCharScalarImageType*>(output));
    }
  else if(dynamic_cast<const FloatVectorImageType*>(input))
    {
    std::cout << "Deep copying FloatVectorImageType" << std::endl;
    ITKHelpers::DeepCopy<FloatVectorImageType>(dynamic_cast<const FloatVectorImageType*>(input), dynamic_cast<FloatVectorImageType*>(output));
    }
  else if(dynamic_cast<const FloatVector2ImageType*>(input))
    {
    std::cout << "Deep copying FloatVector2ImageType" << std::endl;
    ITKHelpers::DeepCopy<FloatVector2ImageType>(dynamic_cast<const FloatVector2ImageType*>(input), dynamic_cast<FloatVector2ImageType*>(output));
    }
  else if(dynamic_cast<const IntImageType*>(input))
    {
    std::cout << "Deep copying IntImageType" << std::endl;
    ITKHelpers::DeepCopy<IntImageType>(dynamic_cast<const IntImageType*>(input), dynamic_cast<IntImageType*>(output));
    }
  else
    {
    std::cout << "Image is Invalid type!" << std::endl;
    std::cerr << "Cannot cast to any of the specified types!" << std::endl;
    }
}*/

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

} // end namespace
