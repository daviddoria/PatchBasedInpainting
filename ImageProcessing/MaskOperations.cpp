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

#include "MaskOperations.h"

#include <stdexcept>

namespace MaskOperations
{
  
itk::Index<2> FindPixelAcrossHole(const itk::Index<2>& queryPixel, const FloatVector2Type& inputDirection, const Mask* const mask)
{
  if(!mask->IsValid(queryPixel))
    {
    throw std::runtime_error("Can only follow valid pixel+vector across a hole.");
    }

  // Determine if 'direction' is pointing inside or outside the hole

  FloatVector2Type direction = inputDirection;

  itk::Index<2> nextPixelAlongVector = ITKHelpers::GetNextPixelAlongVector(queryPixel, direction);

  // If the next pixel along the isophote is in bounds and in the hole region of the patch, procede.
  if(mask->GetLargestPossibleRegion().IsInside(nextPixelAlongVector) && mask->IsHole(nextPixelAlongVector))
    {
    // do nothing
    }
  else
    {
    // There is no requirement for the isophote to be pointing a particular orientation, so try to step along the negative isophote.
    direction *= -1.0;
    nextPixelAlongVector = ITKHelpers::GetNextPixelAlongVector(queryPixel, direction);
    }

  // Trace across the hole
  while(mask->IsHole(nextPixelAlongVector))
    {
    nextPixelAlongVector = ITKHelpers::GetNextPixelAlongVector(nextPixelAlongVector, direction);
    if(!mask->GetLargestPossibleRegion().IsInside(nextPixelAlongVector))
      {
      throw std::runtime_error("Helpers::FindPixelAcrossHole could not find a valid neighbor!");
      }
    }

  return nextPixelAlongVector;
}


void VectorMaskedBlur(const FloatVectorImageType* const inputImage, const Mask::Pointer mask, const float blurVariance, FloatVectorImageType::Pointer output)
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

  ITKHelpers::DeepCopy<FloatVectorImageType>(imageToVectorImageFilter->GetOutput(), output);
}

} // end namespace
