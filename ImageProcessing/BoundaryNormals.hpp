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
  ComputeBoundaryNormals(boundaryNormalsImage, maskBlurVariance, this->MaskImage->GetLargestPossibleRegion());
}

template <typename TNormalsImage>
void BoundaryNormals::ComputeBoundaryNormals(TNormalsImage* const boundaryNormalsImage, const float maskBlurVariance,
                                             const itk::ImageRegion<2>& region)
{
  // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

  // Compute the boundary of the mask
  typedef itk::Image<unsigned char, 2> BoundaryImageType;
  BoundaryImageType::Pointer boundaryImage = BoundaryImageType::New();
  this->MaskImage->CreateBoundaryImageInRegion(region, boundaryImage, Mask::VALID, 255);

  if(this->IsDebugOn())
  {
    ITKHelpers::WriteImage(boundaryImage.GetPointer(),
                           "ComputeBoundaryNormals.BoundaryImage.mha");
  }

  // Blur the mask so that the normals are not quantized so much. Also, pixels with only diagonal
  // valid neighbors have undefined gradients without this blurring.

  ITKHelpers::FloatScalarImageType::Pointer blurredMask = ITKHelpers::FloatScalarImageType::New();
  blurredMask->SetRegions(region);
  blurredMask->Allocate();

  ITKHelpers::BlurAllChannelsInRegion(this->MaskImage, blurredMask.GetPointer(), maskBlurVariance, region);

  if(this->GetDebugImages())
  {
    ITKHelpers::WriteImage(blurredMask.GetPointer(),
                            "ComputeBoundaryNormals.BlurredMask.mha");
  }

  // Compute the gradient of the blurred mask
  ITKHelpers::ComputeGradientsInRegion(blurredMask.GetPointer(), region, boundaryNormalsImage);

  if(this->GetDebugImages())
  {
    ITKHelpers::WriteImage(boundaryNormalsImage,
                          "ComputeBoundaryNormals.BoundaryNormals.mha");
  }

  // Only keep the normals at the boundary
  itk::ImageRegionIteratorWithIndex<TNormalsImage>
      normalsImageIterator(boundaryNormalsImage, boundaryNormalsImage->GetLargestPossibleRegion());

  typename TNormalsImage::PixelType zeroNormal;
  zeroNormal.Fill(0);

  while(!normalsImageIterator.IsAtEnd())
  {
    if(boundaryImage->GetPixel(normalsImageIterator.GetIndex()))
    {
      typename TNormalsImage::PixelType normal = normalsImageIterator.Get();
      normal.Normalize();
      normalsImageIterator.Set(normal);
    }
    else
    {
      normalsImageIterator.Set(zeroNormal);
    }

    ++normalsImageIterator;
  }

  if(this->GetDebugImages())
  {
    ITKHelpers::WriteImage(boundaryNormalsImage,
                          "ComputeBoundaryNormals.BoundaryNormalsPruned.mha");
  }

}

#endif
