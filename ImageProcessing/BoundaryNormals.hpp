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

#ifndef BoundaryNormals_HPP
#define BoundaryNormals_HPP

//#include "BoundaryNormals.h"

// ITK
#include "itkMaskImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkGradientImageFilter.h"

// Submodules
#include <ITKHelpers/ITKHelpers.h>

template <typename TNormalsImage>
void BoundaryNormals::ComputeBoundaryNormals(TNormalsImage* const boundaryNormalsImage, const float maskBlurVariance)
{
  // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

  // Compute the boundary of the mask
  typedef itk::Image<unsigned char, 2> BoundaryImageType;
  BoundaryImageType::Pointer boundaryImage = BoundaryImageType::New();
  this->MaskImage->FindBoundary(boundaryImage, Mask::VALID, 255);

  if(this->IsDebugOn())
  {
    ITKHelpers::WriteImage(boundaryImage.GetPointer(),
                           "ComputeBoundaryNormals.BoundaryImage.mha");
  }

  // Blur the mask so that the normals are not quantized so much. Also, pixels with only diagonal
  // valid neighbors have undefined gradients without this blurring.
  typedef itk::DiscreteGaussianImageFilter<Mask, ITKHelpers::FloatScalarImageType>  BlurFilterType;
  BlurFilterType::Pointer gaussianFilter = BlurFilterType::New();
  gaussianFilter->SetInput(this->MaskImage);
  gaussianFilter->SetVariance(maskBlurVariance);
  gaussianFilter->Update();

  if(this->IsDebugOn())
  {
     ITKHelpers::WriteImage(gaussianFilter->GetOutput(),
                            "ComputeBoundaryNormals.BlurredMask.mha");
  }

  // Compute the gradient of the blurred mask
  typedef itk::GradientImageFilter<ITKHelpers::FloatScalarImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  if(this->IsDebugOn())
  {
    ITKHelpers::WriteImage(gradientFilter->GetOutput(),
                          "ComputeBoundaryNormals.BlurredMaskGradient.mha");
  }

  // Only keep the normals at the boundary
  typedef itk::MaskImageFilter<TNormalsImage, ITKHelpers::UnsignedCharScalarImageType, TNormalsImage> MaskFilterType;
  typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(gradientFilter->GetOutput());
  maskFilter->SetMaskImage(boundaryImage);
  maskFilter->Update();

  if(this->IsDebugOn())
  {
    ITKHelpers::WriteImage(maskFilter->GetOutput(),
                           "ComputeBoundaryNormals.BoundaryNormalsUnnormalized.mha");
  }

  // Allocate the image to return
//  ITKHelpers::DeepCopy(maskFilter->GetOutput(), boundaryNormalsImage);
  ITKHelpers::InitializeImage(boundaryNormalsImage, this->MaskImage->GetLargestPossibleRegion());

  // Normalize the vectors because we just care about their direction
  std::vector<itk::Index<2> > boundaryPixels = ITKHelpers::GetNonZeroPixels(boundaryImage.GetPointer());

  for(std::vector<itk::Index<2> >::const_iterator boundaryPixelIterator = boundaryPixels.begin();
      boundaryPixelIterator != boundaryPixels.end(); ++boundaryPixelIterator)
  {
    typename TNormalsImage::PixelType p = maskFilter->GetOutput()->GetPixel(*boundaryPixelIterator);
    p.Normalize();
    boundaryNormalsImage->SetPixel(*boundaryPixelIterator, p);
  }

}

#endif
