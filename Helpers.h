/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HELPERS_H
#define HELPERS_H

#include "Types.h"

#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
#include "itkPasteImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkPasteImageFilter.h"

///////// Declarations ///////////
template <class T>
void CreateConstantPatch(typename T::Pointer patch, typename T::PixelType value, unsigned int radius);

template<typename T>
void ReplaceValue(typename T::Pointer image, typename T::PixelType queryValue, typename T::PixelType replacementValue);

template<typename T>
void WriteImage(typename T::Pointer image, std::string filename);

template <class T>
void CopyPatchIntoImage(typename T::Pointer patch, typename T::Pointer image, itk::Index<2> position);

template <class T>
void CreateBlankPatch(typename T::Pointer patch, unsigned int radius);

template <>
void CreateBlankPatch<UnsignedCharImageType>(UnsignedCharImageType::Pointer patch, unsigned int radius);

template <class T>
float MaxValue(typename T::Pointer image);

template <class T>
float MaxValueLocation(typename T::Pointer image);

template <class T>
float MinValue(typename T::Pointer image);

template <class T>
itk::Index<2> MinValueLocation(typename T::Pointer image);

template <class T>
void CopyPatchIntoTargetRegion(typename T::Pointer patch, typename T::Pointer image, itk::Index<2> position);

// Non template function declarations
itk::ImageRegion<2> GetRegionAroundPixel(itk::Index<2> pixel, unsigned int radius);

///////// Definitions ///////////
template<typename T>
void ReplaceValue(typename T::Pointer image, typename T::PixelType queryValue, typename T::PixelType replacementValue)
{
  // This function replaces all pixels in 'image' equal to 'queryValue' with 'replacementValue'
  itk::ImageRegionIterator<T> imageIterator(image, image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() == queryValue)
      {
      imageIterator.Set(replacementValue);
      }
    ++imageIterator;
    }
}

template<typename T>
void WriteImage(typename T::Pointer image, std::string filename)
{
  // This is a convenience function so that images can be written in 1 line instead of 4.
  typename itk::ImageFileWriter<T>::Pointer writer = itk::ImageFileWriter<T>::New();
  writer->SetFileName(filename);
  writer->SetInput(image);
  writer->Update();
}


template <class T>
void CreateBlankPatch(typename T::Pointer patch, unsigned int radius)
{
  CreateConstantPatch(patch, typename T::PixelType::Zero(), radius);
}

template <class T>
void CreateConstantPatch(typename T::Pointer patch, typename T::PixelType value, unsigned int radius)
{
  typename T::IndexType start;
  start.Fill(0);

  typename T::SizeType size;
  size.Fill(radius*2 + 1);

  typename T::RegionType region(start, size);

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<T> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }
}

template <class T>
float MaxValue(typename T::Pointer image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetMaximum();
}

template <class T>
float MaxValueLocation(typename T::Pointer image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMaximum();
}

template <class T>
float MinValue(typename T::Pointer image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetMinimum();
}

template <class T>
itk::Index<2> MinValueLocation(typename T::Pointer image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMinimum();
}

template <class T>
void CopyPatchIntoImage(typename T::Pointer patch, typename T::Pointer image, UnsignedCharImageType::Pointer mask, itk::Index<2> position)
{
  // 'Mask' must be the same size as 'image'
  if(mask->GetLargestPossibleRegion().GetSize()[0] != image->GetLargestPossibleRegion().GetSize()[0])
    {
    std::cerr << "mask and image must be the same size!" << std::endl;
    exit(-1);
    }
    
  // This function copies 'patch' into 'image' centered at 'position' only where the 'mask' is non-zero
  itk::ImageRegionConstIterator<T> patchIterator(patch,patch->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(mask,GetRegionAroundPixel(position, patch->GetLargestPossibleRegion().GetSize()[0]));
  itk::ImageRegionIterator<T> imageIterator(image, GetRegionAroundPixel(position, patch->GetLargestPossibleRegion().GetSize()[0]));

  while(!patchIterator.IsAtEnd())
    {
    if(maskIterator.Get()) // we are in the target region
      {
      imageIterator.Set(patchIterator.Get());
      }
    ++imageIterator;
    ++maskIterator;
    ++patchIterator;
    }
}


template <class T>
void CopyPatchIntoImage(typename T::Pointer patch, typename T::Pointer image, itk::Index<2> position)
{
  // This function copies 'patch' into 'image' centered at 'position'.

  // The PasteFilter expects the lower left corner of the destination position, but we have passed the center pixel.
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;
  
  typedef itk::PasteImageFilter <T, T> PasteImageFilterType;

  typename PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New();
  pasteFilter->SetInput(0, image);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(position);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();
  
  image->Graft(pasteFilter->GetOutput());
}

#endif