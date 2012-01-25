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

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"
#include "Mask.h"

// ITK
#include "itkGaussianOperator.h"

// STL
#include <stdexcept>

namespace Derivatives
{

template <typename TImage>
void MaskedDerivative(const TImage* const image, const Mask* const mask, const unsigned int direction, FloatScalarImageType* const output)
{
  // Setup the output
  output->SetRegions(image->GetLargestPossibleRegion());
  output->Allocate();
  output->FillBuffer(0);

  itk::ImageRegionConstIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    // We should not compute derivatives for pixels in the hole.
    if(mask->IsHole(imageIterator.GetIndex()))
      {
      ++imageIterator;
      continue;
      }

    // Determine which neighbors are valid
    bool backwardValid = false;
    itk::Index<2> backwardIndex = imageIterator.GetIndex();
    backwardIndex[direction]--;
    if(image->GetLargestPossibleRegion().IsInside(backwardIndex) && mask->IsValid(backwardIndex))
      {
      backwardValid = true;
      }

    bool forwardValid = false;
    itk::Index<2> forwardIndex = imageIterator.GetIndex();
    forwardIndex[direction]++;
    if(image->GetLargestPossibleRegion().IsInside(forwardIndex) && mask->IsValid(forwardIndex))
      {
      forwardValid = true;
      }

    // Compute the correct difference
    float difference = 0.0f;

    if(backwardValid && !forwardValid) // Use backwards half difference
      {
      difference = image->GetPixel(imageIterator.GetIndex()) - image->GetPixel(backwardIndex);
      }
    else if(!backwardValid && forwardValid) // Use forwards half difference
      {
      difference = image->GetPixel(forwardIndex) - image->GetPixel(imageIterator.GetIndex());
      }
    else if(backwardValid && forwardValid) // Use full difference
      {
      difference = (image->GetPixel(forwardIndex) - image->GetPixel(backwardIndex))/2.0f;
      }
    else// if(!backwardValid && !forwardValid) // No valid neighbors in this direction
      {
      difference = 0.0f; // There is nothing we can do here, so set the derivative to zero.
      }

    output->SetPixel(imageIterator.GetIndex(), difference);

    ++imageIterator;
    }
}


template <typename TImage>
void MaskedDerivativePrewitt(const TImage* const image, const Mask* const mask, const unsigned int direction, FloatScalarImageType* const output)
{
  if(direction > 1)
    {
    throw std::runtime_error("This function can only compute derivatives of 2D images!");
    }

  // Setup the output
  ITKHelpers::InitializeImage<FloatScalarImageType>(output, image->GetLargestPossibleRegion());

  itk::ImageRegionIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

  // If we are taking x derivatives, we want to use 3 columns. If we are taking y derivatives, we want to use 3 rows.
  unsigned int shiftIndex;
  if(direction == 0)
    {
    shiftIndex = 1;
    }
  else
    {
    shiftIndex = 0;
    }

  while(!imageIterator.IsAtEnd())
    {
    // We should not compute derivatives for pixels in the hole.
    if(mask->IsHole(imageIterator.GetIndex()))
      {
      ++imageIterator;
      continue;
      }

    float difference = 0.0f;
    unsigned int numberUsed = 0;
    for(int shift = -1; shift <= 1; shift++) // this shift is either rows or columns, depending on the derivative direction
      {
      itk::Index<2> centerIndex;
      centerIndex[direction] = imageIterator.GetIndex()[direction];
      centerIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;
      if(!(image->GetLargestPossibleRegion().IsInside(centerIndex) && mask->IsValid(centerIndex)))
        {
        continue;
        }

      // Determine which neighbors are valid
      bool backwardValid = false;
      itk::Index<2> backwardIndex;
      backwardIndex[direction] = imageIterator.GetIndex()[direction] - 1;
      backwardIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;
      if(image->GetLargestPossibleRegion().IsInside(backwardIndex) && mask->IsValid(backwardIndex))
        {
        backwardValid = true;
        }

      bool forwardValid = false;
      itk::Index<2> forwardIndex;
      forwardIndex[direction] = imageIterator.GetIndex()[direction] + 1;
      forwardIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;

      if(image->GetLargestPossibleRegion().IsInside(forwardIndex) && mask->IsValid(forwardIndex))
        {
        forwardValid = true;
        }

      if(backwardValid && !forwardValid) // Use backwards half difference
        {
        difference += image->GetPixel(centerIndex) - image->GetPixel(backwardIndex);
        numberUsed++;
        }
      else if(!backwardValid && forwardValid) // Use forwards half difference
        {
        difference += image->GetPixel(forwardIndex) - image->GetPixel(centerIndex);
        numberUsed++;
        }
      else if(backwardValid && forwardValid) // Use full difference
        {
        difference += (image->GetPixel(forwardIndex) - image->GetPixel(backwardIndex))/2.0f;
        numberUsed++;
        }
      else// if(!backwardValid && !forwardValid) // No valid neighbors in this direction
        {
        difference += 0.0f; // There is nothing we can do here, so set the derivative to zero.
        }
      } // end shift loop

    if(numberUsed > 1)
      {
      difference /= static_cast<float>(numberUsed);
      }

    output->SetPixel(imageIterator.GetIndex(), difference);

    ++imageIterator;
    }
}


template <typename TImage>
void MaskedDerivativeSobel(const TImage* const image, const Mask* const mask, const unsigned int direction, FloatScalarImageType* const output)
{
  if(direction > 1)
    {
    throw std::runtime_error ("This function can only compute derivatives of 2D images!");
    }

  // Setup the output
  ITKHelpers::InitializeImage<FloatScalarImageType>(output, image->GetLargestPossibleRegion());

  itk::ImageRegionIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

  // If we are taking x derivatives, we want to use 3 columns. If we are taking y derivatives, we want to use 3 rows.
  unsigned int shiftIndex;
  if(direction == 0)
    {
    shiftIndex = 1;
    }
  else
    {
    shiftIndex = 0;
    }

  while(!imageIterator.IsAtEnd())
    {
    // We should not compute derivatives for pixels in the hole.
    if(mask->IsHole(imageIterator.GetIndex()))
      {
      ++imageIterator;
      continue;
      }

    float totalDifference = 0.0f;
    unsigned int numberUsed = 0;
    for(int shift = -1; shift <= 1; shift++) // this shift is either rows or columns, depending on the derivative direction
      {
      itk::Index<2> centerIndex;
      centerIndex[direction] = imageIterator.GetIndex()[direction];
      centerIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;
      if(!(image->GetLargestPossibleRegion().IsInside(centerIndex) && mask->IsValid(centerIndex)))
        {
        continue;
        }

      float difference = 0.0f;

      // Determine which neighbors are valid
      bool backwardValid = false;
      itk::Index<2> backwardIndex;
      backwardIndex[direction] = imageIterator.GetIndex()[direction] - 1;
      backwardIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;
      if(image->GetLargestPossibleRegion().IsInside(backwardIndex) && mask->IsValid(backwardIndex))
        {
        backwardValid = true;
        }

      bool forwardValid = false;
      itk::Index<2> forwardIndex;
      forwardIndex[direction] = imageIterator.GetIndex()[direction] + 1;
      forwardIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;

      if(image->GetLargestPossibleRegion().IsInside(forwardIndex) && mask->IsValid(forwardIndex))
        {
        forwardValid = true;
        }
      unsigned int weight = 1;
      if(shift == 0)
        {
        weight = 2;
        }
      if(backwardValid && !forwardValid) // Use backwards half difference
        {
        difference = image->GetPixel(centerIndex) - image->GetPixel(backwardIndex);
        numberUsed += weight;
        }
      else if(!backwardValid && forwardValid) // Use forwards half difference
        {
        difference = image->GetPixel(forwardIndex) - image->GetPixel(centerIndex);
        numberUsed += weight;
        }
      else if(backwardValid && forwardValid) // Use full difference
        {
        difference = (image->GetPixel(forwardIndex) - image->GetPixel(backwardIndex))/2.0f;
        numberUsed += weight;
        }
      else// if(!backwardValid && !forwardValid) // No valid neighbors in this direction
        {
        difference = 0.0f; // There is nothing we can do here, so set the derivative to zero.
        }

      difference *= static_cast<float>(weight);

      totalDifference += difference;
      } // end shift loop

    if(numberUsed > 1)
      {
      totalDifference /= static_cast<float>(numberUsed);
      }

    output->SetPixel(imageIterator.GetIndex(), totalDifference);

    ++imageIterator;
    }
}

template <typename TImage>
void MaskedDerivativeGaussianInRegion(const TImage* const image, const Mask* const mask, const unsigned int direction,
                                      const itk::ImageRegion<2>& region, FloatScalarImageType* const output)
{
  // It is assumed that 'output' is the right size and initialized already.

  if(direction > 1)
    {
    throw std::runtime_error("This function can only compute derivatives of 2D images!");
    }

  // Create a Gaussian kernel
  typedef itk::GaussianOperator<float, 1> GaussianOperatorType;

  // Make a (2*kernelRadius+1)x1 kernel
  itk::Size<1> radius;
  radius.Fill(5); // Make a length 11 kernel

  GaussianOperatorType gaussianOperator;
  gaussianOperator.SetDirection(0); // It doesn't matter which direction we set - we will be interpreting the kernel as 1D (no direction)
  gaussianOperator.SetVariance(3);
  gaussianOperator.CreateToRadius(radius);

  // Setup the output
  //Helpers::InitializeImage<FloatScalarImageType>(output, image->GetLargestPossibleRegion());

  itk::ImageRegionConstIterator<TImage> imageIterator(image, region);

  // If we are taking x derivatives, we want to use 3 columns. If we are taking y derivatives, we want to use 3 rows.
  unsigned int shiftIndex;
  if(direction == 0)
    {
    shiftIndex = 1;
    }
  else
    {
    shiftIndex = 0;
    }

  while(!imageIterator.IsAtEnd())
    {
    // We should not compute derivatives for pixels in the hole.
    if(mask->IsHole(imageIterator.GetIndex()))
      {
      ++imageIterator;
      continue;
      }

    float totalDifference = 0.0f;
    float totalWeight = 0.0f;
    for(unsigned int shiftId = 0; shiftId < gaussianOperator.Size(); shiftId++) // this shift is either rows or columns, depending on the derivative direction
      {
      int shift = gaussianOperator.GetOffset(shiftId)[0];

      itk::Index<2> centerIndex;
      centerIndex[direction] = imageIterator.GetIndex()[direction];
      centerIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;
      if(!(image->GetLargestPossibleRegion().IsInside(centerIndex) && mask->IsValid(centerIndex)))
        {
        continue;
        }

      float difference = 0.0f;

      // Determine which neighbors are valid
      bool backwardValid = false;
      itk::Index<2> backwardIndex;
      backwardIndex[direction] = imageIterator.GetIndex()[direction] - 1;
      backwardIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;
      if(image->GetLargestPossibleRegion().IsInside(backwardIndex) && mask->IsValid(backwardIndex))
        {
        backwardValid = true;
        }

      bool forwardValid = false;
      itk::Index<2> forwardIndex;
      forwardIndex[direction] = imageIterator.GetIndex()[direction] + 1;
      forwardIndex[shiftIndex] = imageIterator.GetIndex()[shiftIndex] + shift;

      if(image->GetLargestPossibleRegion().IsInside(forwardIndex) && mask->IsValid(forwardIndex))
        {
        forwardValid = true;
        }

      float weight = gaussianOperator.GetElement(shiftId);

      if(backwardValid && !forwardValid) // Use backwards half difference
        {
        difference = image->GetPixel(centerIndex) - image->GetPixel(backwardIndex);
        totalWeight += weight;
        }
      else if(!backwardValid && forwardValid) // Use forwards half difference
        {
        difference = image->GetPixel(forwardIndex) - image->GetPixel(centerIndex);
        totalWeight += weight;
        }
      else if(backwardValid && forwardValid) // Use full difference
        {
        difference = (image->GetPixel(forwardIndex) - image->GetPixel(backwardIndex))/2.0f;
        totalWeight += weight;
        }
      else// if(!backwardValid && !forwardValid) // No valid neighbors in this direction
        {
        difference = 0.0f; // There is nothing we can do here, so set the derivative to zero.
        }

      difference *= weight;
      totalDifference += difference;
      totalWeight += weight;
      } // end shift loop

    if(totalWeight > 0.0f)
      {
      totalDifference /= totalWeight;
      }

    output->SetPixel(imageIterator.GetIndex(), totalDifference);

    ++imageIterator;
    }
}

template <typename TImage>
void MaskedDerivativeGaussian(const TImage* const image, const Mask* const mask, const unsigned int direction, FloatScalarImageType* const output)
{
  MaskedDerivativeGaussianInRegion(image, mask, direction, image->GetLargestPossibleRegion(), output);
}

template <typename TImage>
void MaskedGradientInRegion(const TImage* const image, const Mask* const mask, const itk::ImageRegion<2>& region, FloatVector2ImageType* const output)
{
  // Compute the derivatives
  // X derivative
  FloatScalarImageType::Pointer xDerivative = FloatScalarImageType::New();
  ITKHelpers::InitializeImage<FloatScalarImageType>(xDerivative, image->GetLargestPossibleRegion());

  //Helpers::MaskedDerivative<FloatScalarImageType>(image, mask, 0, xDerivative);
  //Helpers::MaskedDerivativePrewitt<FloatScalarImageType>(image, mask, 0, xDerivative);
  //Helpers::MaskedDerivativeSobel<FloatScalarImageType>(image, mask, 0, xDerivative);
  MaskedDerivativeGaussianInRegion<TImage>(image, mask, 0, region, xDerivative);
  //Helpers::DebugWriteImageConditional<FloatScalarImageType>(xDerivative, "Debug/ComputeMaskedIsophotes.xderivative.mha", this->DebugImages);

  // Y derivative
  FloatScalarImageType::Pointer yDerivative = FloatScalarImageType::New();
  ITKHelpers::InitializeImage<FloatScalarImageType>(yDerivative, image->GetLargestPossibleRegion());

  //Helpers::MaskedDerivative<FloatScalarImageType>(image, mask, 1, yDerivative);
  //Helpers::MaskedDerivativePrewitt<FloatScalarImageType>(image, mask, 1, yDerivative);
  //Helpers::MaskedDerivativeSobel<FloatScalarImageType>(image, mask, 1, yDerivative);
  MaskedDerivativeGaussianInRegion<TImage>(image, mask, 1, region, yDerivative);
  //Helpers::DebugWriteImageConditional<FloatScalarImageType>(yDerivative, "Debug/ComputeMaskedIsophotes.yderivative.mha", this->DebugImages);

  // Combine derivatives
  GradientFromDerivatives<float>(xDerivative, yDerivative, output);
}

template <typename TImage>
void MaskedGradient(const TImage* const image, const Mask* const mask, FloatVector2ImageType* const output)
{
  MaskedGradientInRegion<TImage>(image, mask, image->GetLargestPossibleRegion(), output);
}

template<typename TPixel>
void GradientFromDerivativesInRegion(const itk::Image<TPixel, 2>* const xDerivative, const itk::Image<TPixel, 2>* const yDerivative,
                                     const itk::ImageRegion<2>& region, itk::Image<itk::CovariantVector<TPixel, 2> >* const output)
{
  // It is assumed that 'output' is the right size and initialized already.

  if(xDerivative->GetLargestPossibleRegion() != yDerivative->GetLargestPossibleRegion())
    {
    std::cerr << "X and Y derivative images must be the same size!" << std::endl;
    return;
    }

  itk::ImageRegionIterator<itk::Image<itk::CovariantVector<TPixel, 2> > > imageIterator(output, region);

  while(!imageIterator.IsAtEnd())
    {
    itk::CovariantVector<TPixel, 2> vectorPixel;
    vectorPixel[0] = xDerivative->GetPixel(imageIterator.GetIndex());
    vectorPixel[1] = yDerivative->GetPixel(imageIterator.GetIndex());

    output->SetPixel(imageIterator.GetIndex(), vectorPixel);

    ++imageIterator;
    }
}

template<typename TPixel>
void GradientFromDerivatives(const itk::Image<TPixel, 2>* const xDerivative, const itk::Image<TPixel, 2>* const yDerivative,
                             itk::Image<itk::CovariantVector<TPixel, 2> >* const output)
{
  GradientFromDerivativesInRegion<TPixel>(xDerivative, yDerivative, xDerivative->GetLargestPossibleRegion(), output);
}

} // end namespace
