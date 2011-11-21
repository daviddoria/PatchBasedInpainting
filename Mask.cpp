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
#include "Helpers.h"

// ITK
#include "itkBinaryContourImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkInvertIntensityImageFilter.h"

Mask::Mask()
{
  this->HoleValue = 255;
  this->ValidValue = 0;
}

std::vector<itk::Index<2> > Mask::GetValidPixelsInRegion(const itk::ImageRegion<2>& inputRegion)
{
  itk::ImageRegion<2> region = inputRegion;
  region.Crop(this->GetLargestPossibleRegion());
  
  std::vector<itk::Index<2> > validPixels;

  itk::ImageRegionIterator<Mask> iterator(this, region);

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

std::vector<itk::Index<2> > Mask::GetHolePixelsInRegion(const itk::ImageRegion<2>& inputRegion)
{
  itk::ImageRegion<2> region = inputRegion;
  region.Crop(this->GetLargestPossibleRegion());
  
  std::vector<itk::Index<2> > holePixels;

  typename itk::ImageRegionIterator<Mask> iterator(this, region);

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

void Mask::DeepCopyFrom(const Mask::Pointer inputMask)
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

void Mask::ExpandHole()
{
  // Expand the mask - this is necessary to prevent the isophotes from being undefined in the target region
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(2); // Just a little bit of expansion
  //radius.Fill(this->PatchRadius[0]); // This was working, but huge expansion
  //radius.Fill(2.0* this->PatchRadius[0]);

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<Mask, Mask, StructuringElementType> BinaryDilateImageFilterType;
  BinaryDilateImageFilterType::Pointer expandMaskFilter = BinaryDilateImageFilterType::New();
  expandMaskFilter->SetInput(this);
  expandMaskFilter->SetKernel(structuringElement);
  expandMaskFilter->Update();
    
  //Helpers::DeepCopy<Mask>(expandMaskFilter->GetOutput(), this->CurrentMask);
  this->DeepCopyFrom(expandMaskFilter->GetOutput());

}


void Mask::FindBoundary(UnsignedCharScalarImageType::Pointer boundaryImage)
{
  EnterFunction("Mask::FindBoundary()");
  try
  {
    // Compute the "outer" boundary of the region to fill. That is, we want the boundary pixels to be in the source region.

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
    // The BinaryContourImageFilter will change all non-boundary pixels to the background color, so the resulting output will be inverted -
    // the boundary pixels will be black and the non-boundary pixels will be white.

    // Find the boundary
    typedef itk::BinaryContourImageFilter <Mask, Mask> binaryContourImageFilterType;
    binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New();
    binaryContourFilter->SetInput(holeOnly);
    binaryContourFilter->SetFullyConnected(true);
    binaryContourFilter->SetForegroundValue(holeOnly->GetValidValue());
    binaryContourFilter->SetBackgroundValue(holeOnly->GetHoleValue());
    binaryContourFilter->Update();

    //HelpersOutput::WriteImageConditional<Mask>(binaryContourFilter->GetOutput(), "Debug/FindBoundary.Boundary.mha", this->DebugImages);
    //HelpersOutput::WriteImageConditional<Mask>(binaryContourFilter->GetOutput(), "Debug/FindBoundary.Boundary.png", this->DebugImages);

    // Since we want to interpret non-zero pixels as boundary pixels, we must invert the image.
    typedef itk::InvertIntensityImageFilter <Mask> InvertIntensityImageFilterType;
    InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
    invertIntensityFilter->SetInput(binaryContourFilter->GetOutput());
    invertIntensityFilter->SetMaximum(255);
    invertIntensityFilter->Update();

    //this->BoundaryImage = binaryContourFilter->GetOutput();
    //this->BoundaryImage->Graft(binaryContourFilter->GetOutput());
    Helpers::DeepCopy<UnsignedCharScalarImageType>(invertIntensityFilter->GetOutput(), boundaryImage);

    //HelpersOutput::WriteImageConditional<UnsignedCharScalarImageType>(this->BoundaryImage, "Debug/FindBoundary.BoundaryImage.mha", this->DebugImages);
    LeaveFunction("FindBoundary()");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindBoundary!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

bool Mask::HasHoleNeighbor(const itk::Index<2>& pixel)
{
  std::vector<itk::Offset<2> > offsets = Helpers::Get8NeighborOffsets();

  for(unsigned int i = 0; i < offsets.size(); ++i)
    {
    if(this->IsHole(pixel + offsets[i]))
      {
      return true;
      }
    }

  return false;
}

itk::Index<2> Mask::FindPixelAcrossHole(const itk::Index<2>& queryPixel, const FloatVector2Type& inputDirection)
{
  if(!this->IsValid(queryPixel))
    {
    std::cerr << "Can only follow valid pixel+vector across a hole." << std::endl;
    exit(-1);
    }

  // Determine if 'direction' is pointing inside or outside the hole
  
  FloatVector2Type direction = inputDirection;

  itk::Index<2> nextPixelAlongVector = Helpers::GetNextPixelAlongVector(queryPixel, direction);

  // If the next pixel along the isophote is in bounds and in the hole region of the patch, procede.
  if(this->GetLargestPossibleRegion().IsInside(nextPixelAlongVector) && this->IsHole(nextPixelAlongVector))
    {
    // do nothing
    }
  else
    {
    // There is no requirement for the isophote to be pointing a particular orientation, so try to step along the negative isophote.
    direction *= -1.0;
    nextPixelAlongVector = Helpers::GetNextPixelAlongVector(queryPixel, direction);
    }
  
  // Trace across the hole
  while(this->IsHole(nextPixelAlongVector))
    {
    nextPixelAlongVector = Helpers::GetNextPixelAlongVector(nextPixelAlongVector, direction);
    if(!this->GetLargestPossibleRegion().IsInside(nextPixelAlongVector))
      {
      std::cerr << "Mask::FindPixelAcrossHole could not find a valid neighbor!" << std::endl;
      exit(-1);
      }
    }
  
  return nextPixelAlongVector;
}
