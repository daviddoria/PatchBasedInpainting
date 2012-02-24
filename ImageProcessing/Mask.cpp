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

#include "Mask.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"

// VTK
#include <vtkImageData.h>

// ITK
#include "itkBinaryContourImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkImageRegionIterator.h"

Mask::Mask()
{
  this->HoleValue = 255;
  this->ValidValue = 0;
}

void Mask::Read(const std::string& filename)
{
  // Ensure the input image can be interpreted as a mask.
  {
  typedef itk::VectorImage<float, 2> TestImageType;
  typedef  itk::ImageFileReader<TestImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(filename);
  imageReader->Update();

  unsigned int numberOfComponents = imageReader->GetOutput()->GetNumberOfComponentsPerPixel();
  if(!(numberOfComponents == 1 || numberOfComponents == 3))
    {
    std::stringstream ss;
    ss << "Number of components for a mask must be 1 or 3! (" << filename << " is " << numberOfComponents << ")";
    throw std::runtime_error(ss.str());
    }
  }

  // Should probably check that all 3 components are the same for all pixels (if numberOfComponents == 3)
  typedef itk::ImageFileReader<Mask> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(filename);
  imageReader->Update();
  
  DeepCopyFrom(imageReader->GetOutput());
}

unsigned int Mask::CountHolePixels(const itk::ImageRegion<2>& region) const
{
  return GetHolePixelsInRegion(region).size();
}

unsigned int Mask::CountHolePixels() const
{
  return CountHolePixels(this->GetLargestPossibleRegion());
}

unsigned int Mask::CountValidPixels(const itk::ImageRegion<2>& region) const
{
  return GetValidPixelsInRegion(region).size();
}

unsigned int Mask::CountValidPixels() const
{
  return CountValidPixels(this->GetLargestPossibleRegion());
}

std::vector<itk::Offset<2> > Mask::GetValidOffsetsInRegion(itk::ImageRegion<2> region) const
{
  region.Crop(this->GetLargestPossibleRegion());

  std::vector<itk::Offset<2> > validOffsets;

  itk::ImageRegionConstIterator<Mask> iterator(this, region);

  while(!iterator.IsAtEnd())
    {
    if(this->IsValid(iterator.GetIndex()))
      {
      validOffsets.push_back(iterator.GetIndex() - region.GetIndex());
      }

    ++iterator;
    }
  return validOffsets;
}

std::vector<itk::Offset<2> > Mask::GetHoleOffsetsInRegion(itk::ImageRegion<2> region) const
{
  region.Crop(this->GetLargestPossibleRegion());

  std::vector<itk::Offset<2> > holeOffsets;

  itk::ImageRegionConstIterator<Mask> iterator(this, region);

  while(!iterator.IsAtEnd())
    {
    if(this->IsHole(iterator.GetIndex()))
      {
      holeOffsets.push_back(iterator.GetIndex() - region.GetIndex());
      }

    ++iterator;
    }
  return holeOffsets;
}

std::vector<itk::Index<2> > Mask::GetValidPixelsInRegion(itk::ImageRegion<2> region) const
{
  region.Crop(this->GetLargestPossibleRegion());

  std::vector<itk::Index<2> > validPixels;

  itk::ImageRegionConstIterator<Mask> iterator(this, region);

  while(!iterator.IsAtEnd())
    {
    if(this->IsValid(iterator.GetIndex()))
      {
      validPixels.push_back(iterator.GetIndex());
      }

    ++iterator;
    }
  return validPixels;
}

std::vector<itk::Index<2> > Mask::GetHolePixelsInRegion(itk::ImageRegion<2> region) const
{
  region.Crop(this->GetLargestPossibleRegion());

  std::vector<itk::Index<2> > holePixels;

  typename itk::ImageRegionConstIterator<Mask> iterator(this, region);

  while(!iterator.IsAtEnd())
    {
    if(this->IsHole(iterator.GetIndex()))
      {
      holePixels.push_back(iterator.GetIndex());
      }

    ++iterator;
    }
  return holePixels;
}

bool Mask::IsHole(const itk::Index<2>& index) const
{
  if(this->GetPixel(index) == this->HoleValue)
    {
    return true;
    }
  return false;
}

bool Mask::IsHole(const itk::ImageRegion<2>& region) const
{
  // If any of the pixels in the region are not hole pixels, the region is not entirely hole pixels.

  itk::ImageRegionConstIterator<Mask> maskIterator(this, region);

  while(!maskIterator.IsAtEnd())
    {
    if(!this->IsHole(maskIterator.GetIndex()))
      {
      return false;
      }

    ++maskIterator;
    }
  return true;
}

bool Mask::IsValid(const itk::ImageRegion<2>& region) const
{
  // If any of the pixels in the region are invalid, the region is invalid.

  itk::ImageRegionConstIterator<Mask> maskIterator(this, region);

  while(!maskIterator.IsAtEnd())
    {
    if(!this->IsValid(maskIterator.GetIndex()))
      {
      //std::cout << "Mask::IsValid - Pixel " << maskIterator.GetIndex() << " has value " << static_cast<unsigned int>(maskIterator.Get())
      //          << " which makes the region invalid because Mask::ValidValue = " << static_cast<unsigned int>(this->ValidValue) << std::endl;
      return false;
      }

    ++maskIterator;
    }
  return true;
}

bool Mask::IsValid(const itk::Index<2>& index) const
{
  if(this->GetPixel(index) == this->ValidValue)
    {
    return true;
    }
  return false;
}

void Mask::Invert()
{
  // Exchange HoleValue and ValidValue, but leave everything else alone.
  itk::ImageRegionIterator<Mask> maskIterator(this, this->GetLargestPossibleRegion());
  unsigned int invertedCounter = 0;
  while(!maskIterator.IsAtEnd())
    {
    if(this->IsValid(maskIterator.GetIndex()))
      {
      maskIterator.Set(this->HoleValue);
      invertedCounter++;
      }
    else if(this->IsHole(maskIterator.GetIndex()))
      {
      maskIterator.Set(this->ValidValue);
      invertedCounter++;
      }
    ++maskIterator;
    }
  //std::cout << "Inverted " << invertedCounter << " in the mask." << std::endl;
}

void Mask::Cleanup()
{
  // We want to interpret pixels that are "pretty much hole value" as holes, and pixels that
  // are "pretty much valid value" as valid. The "do not use" pixels must be very far away from both of these values.
  itk::ImageRegionIterator<Mask> maskIterator(this, this->GetLargestPossibleRegion());

  float tolerance = 4;
  while(!maskIterator.IsAtEnd())
    {
    if(fabs(maskIterator.Get() - this->ValidValue) < tolerance)
      {
      //std::cout << "Setting valid pixel to " << static_cast<unsigned int>(this->ValidValue) << std::endl;
      maskIterator.Set(this->ValidValue);
      }
    else if(fabs(maskIterator.Get() - this->HoleValue) < tolerance)
      {
      //std::cout << "Setting hole pixel to " << static_cast<unsigned int>(this->HoleValue) << std::endl;
      maskIterator.Set(this->HoleValue);
      }
    ++maskIterator;
    }

}

void Mask::SetHoleValue(const unsigned char value)
{
  this->HoleValue = value;
}

void Mask::SetValidValue(const unsigned char value)
{
  this->ValidValue = value;
}

unsigned char Mask::GetHoleValue() const
{
  return this->HoleValue;
}

unsigned char Mask::GetValidValue() const
{
  return this->ValidValue;
}

void Mask::OutputMembers() const
{
  std::cout << "HoleValue: " << static_cast<unsigned int>(this->HoleValue) << std::endl;
  std::cout << "ValidValue: " << static_cast<unsigned int>(this->ValidValue) << std::endl;
}

void Mask::DeepCopyFrom(const Mask* const inputMask)
{
  this->SetRegions(inputMask->GetLargestPossibleRegion());
  this->Allocate();

  itk::ImageRegionConstIterator<Mask> inputIterator(inputMask, inputMask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<Mask> thisIterator(this, this->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    thisIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++thisIterator;
    }
  this->SetHoleValue(inputMask->GetHoleValue());
  this->SetValidValue(inputMask->GetValidValue());
}

void Mask::CopyHolesFrom(const Mask* const inputMask)
{
  itk::ImageRegionConstIterator<Mask> inputIterator(inputMask, inputMask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<Mask> thisIterator(this, this->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    if(inputMask->IsHole(inputIterator.GetIndex()))
      {
      thisIterator.Set(this->HoleValue);
      }
    ++inputIterator;
    ++thisIterator;
    }
}

void Mask::ExpandHole(const unsigned int kernelRadius)
{
  UnsignedCharImageType::Pointer binaryHoleImage = UnsignedCharImageType::New();
  this->CreateBinaryHoleImage(binaryHoleImage);

//   std::cout << "binaryHoleImage: " << std::endl;
//   ITKHelpers::PrintImage(binaryHoleImage.GetPointer());

  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(kernelRadius); // This is correct that the RadiusType expects the region radius, not the side length.

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<UnsignedCharImageType, UnsignedCharImageType, StructuringElementType> BinaryDilateImageFilterType;
  BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
  dilateFilter->SetInput(binaryHoleImage);
  dilateFilter->SetKernel(structuringElement);
  dilateFilter->Update();

//   std::cout << "dilateFilter output: " << std::endl;
//   ITKHelpers::PrintImage(dilateFilter->GetOutput());
  
  // There will now be more hole pixels than there were previously. Copy them into the mask.
  this->CopyHolesFromValue(dilateFilter->GetOutput(), 255);
}

void Mask::ShrinkHole(const unsigned int kernelRadius)
{
  UnsignedCharImageType::Pointer binaryHoleImage = UnsignedCharImageType::New();
  this->CreateBinaryHoleImage(binaryHoleImage);

//   std::cout << "binaryHoleImage: " << std::endl;
//   ITKHelpers::PrintImage(binaryHoleImage.GetPointer());
  
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(kernelRadius); // This is correct that the RadiusType expects the region radius, not the side length.

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryErodeImageFilter<UnsignedCharImageType, UnsignedCharImageType, StructuringElementType> BinaryErodeImageFilterType;
  BinaryErodeImageFilterType::Pointer erodeFilter = BinaryErodeImageFilterType::New();
  erodeFilter->SetInput(binaryHoleImage);
  erodeFilter->SetKernel(structuringElement);
  erodeFilter->Update();

//   std::cout << "erodeFilter output: " << std::endl;
//   ITKHelpers::PrintImage(erodeFilter->GetOutput());
  
  // There will now be more valid pixels than there were previously. Copy them into the mask.
  this->CopyValidPixelsFromValue(erodeFilter->GetOutput(), 0);
}

void Mask::CreateBinaryHoleImage(UnsignedCharImageType* const binaryHoleImage)
{
  binaryHoleImage->SetRegions(this->GetLargestPossibleRegion());
  binaryHoleImage->Allocate();

  itk::ImageRegionIterator<UnsignedCharImageType> binaryImageIterator(binaryHoleImage, binaryHoleImage->GetLargestPossibleRegion());
  // This should result in a white hole on a black background
  while(!binaryImageIterator.IsAtEnd())
    {
    if(this->IsHole(binaryImageIterator.GetIndex()))
      {
      //std::cout << "Hole pixel." << std::endl;
      binaryImageIterator.Set(255);
      }
    else
      {
      binaryImageIterator.Set(0);
      }
    ++binaryImageIterator;
    }

//   std::cout << "CreateBinaryHoleImage()::binaryHoleImage: " << std::endl;
//   ITKHelpers::PrintImage(binaryHoleImage);
}

void Mask::FindBoundaryInRegion(const itk::ImageRegion<2>& region, BoundaryImageType* const boundaryImage) const
{
  // TODO: Make this only compute the boundary in the specified 'region'. Maybe the binary image needs to be computed
  // in a slightly dilated version of the region?
  // TODO: Specify the output value of the boundary (i.e. is it 1? 255? etc)

  // Compute the "outer" boundary of the region to fill. That is,
  // we want the boundary pixels to be in the source region.

  //HelpersOutput::WriteImageConditional<Mask>(this->CurrentMask, "Debug/FindBoundary.CurrentMask.mha", this->DebugImages);
  //HelpersOutput::WriteImageConditional<Mask>(this->CurrentMask, "Debug/FindBoundary.CurrentMask.png", this->DebugImages);

  // Create a binary image (throw away the "dont use" pixels)
  Mask::Pointer holeOnly = Mask::New();
  holeOnly->DeepCopyFrom(this);

  itk::ImageRegionIterator<Mask> maskIterator(holeOnly, holeOnly->GetLargestPossibleRegion());
  // This should result in a white hole on a black background
  while(!maskIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = maskIterator.GetIndex();
    if(!holeOnly->IsHole(currentPixel))
      {
      holeOnly->SetPixel(currentPixel, holeOnly->GetValidValue());
      }
    ++maskIterator;
    }

  //HelpersOutput::WriteImageConditional<Mask>(holeOnly, "Debug/FindBoundary.HoleOnly.mha", this->DebugImages);
  //HelpersOutput::WriteImageConditional<Mask>(holeOnly, "Debug/FindBoundary.HoleOnly.png", this->DebugImages);

  // Since the hole is white, we want the foreground value of the contour filter to be black. This means that the boundary will
  // be detected in the black pixel region, which is on the outside edge of the hole like we want. However,
  // The BinaryContourImageFilter will change all non-boundary pixels to the background color,
  // so the resulting output will be inverted - the boundary pixels will be black and the non-boundary pixels will be white.

  // Find the boundary
  typedef itk::BinaryContourImageFilter <Mask, Mask> binaryContourImageFilterType;
  binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New();
  binaryContourFilter->SetInput(holeOnly);
  binaryContourFilter->SetFullyConnected(true);
  binaryContourFilter->SetForegroundValue(holeOnly->GetValidValue());
  binaryContourFilter->SetBackgroundValue(holeOnly->GetHoleValue());
  binaryContourFilter->Update();

//   OutputHelpers::WriteImageConditional<Mask>(binaryContourFilter->GetOutput(),
//                                              "Debug/FindBoundary.Boundary.mha", this->DebugImages);
//   OutputHelpers::WriteImageConditional<Mask>(binaryContourFilter->GetOutput(),
//                                              "Debug/FindBoundary.Boundary.png", this->DebugImages);

  // Since we want to interpret non-zero pixels as boundary pixels, we must invert the image.
  typedef itk::InvertIntensityImageFilter <Mask> InvertIntensityImageFilterType;
  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(binaryContourFilter->GetOutput());
  invertIntensityFilter->SetMaximum(255);
  invertIntensityFilter->Update();

  //this->BoundaryImage = binaryContourFilter->GetOutput();
  //this->BoundaryImage->Graft(binaryContourFilter->GetOutput());
  ITKHelpers::DeepCopy<UnsignedCharScalarImageType>(invertIntensityFilter->GetOutput(), boundaryImage);

//   OutputHelpers::WriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage,
//                                                                     "Debug/FindBoundary.BoundaryImage.mha", this->DebugImages);

}

void Mask::FindBoundary(UnsignedCharScalarImageType* const boundaryImage) const
{
  FindBoundaryInRegion(this->GetLargestPossibleRegion(), boundaryImage);
}

/** Get a list of the valid neighbors of a pixel.*/
std::vector<itk::Index<2> > Mask::GetValidNeighbors(const itk::Index<2>& pixel) const
{
  return ITKHelpers::Get8NeighborsWithValue(pixel, this, this->ValidValue);
}

bool Mask::HasHoleNeighbor(const itk::Index<2>& pixel) const
{
  if(GetHoleNeighbors(pixel).size() > 0)
    {
    return true;
    }
  return false;
}

/** Get a list of the hole neighbors of a pixel.*/
std::vector<itk::Index<2> > Mask::GetHoleNeighbors(const itk::Index<2>& pixel) const
{
  return ITKHelpers::Get8NeighborsWithValue(pixel, this, this->HoleValue);
}

std::vector<itk::Offset<2> > Mask::GetValidNeighborOffsets(const itk::Index<2>& pixel) const
{
  std::vector<itk::Index<2> > indices = ITKHelpers::Get8NeighborsWithValue(pixel, this, this->ValidValue);
  std::vector<itk::Offset<2> > offsets;
  for(unsigned int i = 0; i < indices.size(); ++i)
  {
    offsets.push_back(indices[i] - pixel);
  }
  return offsets;
}

std::vector<itk::Offset<2> > Mask::GetHoleNeighborOffsets(const itk::Index<2>& pixel) const
{
  std::vector<itk::Index<2> > indices = ITKHelpers::Get8NeighborsWithValue(pixel, this, this->HoleValue);
  std::vector<itk::Offset<2> > offsets;
  for(unsigned int i = 0; i < indices.size(); ++i)
  {
    offsets.push_back(indices[i] - pixel);
  }
  return offsets;
}

void Mask::MarkAsHole(const itk::Index<2>& pixel)
{
  this->SetPixel(pixel, this->HoleValue);
}

void Mask::MarkAsValid(const itk::Index<2>& pixel)
{
  this->SetPixel(pixel, this->ValidValue);
}
