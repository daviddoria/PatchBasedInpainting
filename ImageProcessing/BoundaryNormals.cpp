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

#include "BoundaryNormals.h"

#include "itkMaskImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkGradientImageFilter.h"

#include "ITKHelpers/ITKHelpers.h"

BoundaryNormals::BoundaryNormals(const UnsignedCharScalarImageType* const boundaryImage, const Mask* const mask) :
BoundaryImage(boundaryImage), MaskImage(mask)
{

}

void BoundaryNormals::ComputeBoundaryNormals(const float blurVariance,
                                             ITKHelpers::FloatVector2ImageType* const boundaryNormalsImage)
{
  // Blur the mask, compute the gradient, then keep the normals only at the original mask boundary

//   OutputHelpers::WriteImageConditional(this->BoundaryImage,
//                                        "Debug/ComputeBoundaryNormals.BoundaryImage.mha", this->DebugImages);
  //HelpersOutput::WriteImageConditional(this->MaskImage, "Debug/ComputeBoundaryNormals.CurrentMask.mha",
//                                    this->DebugImages);

  // Blur the mask
  typedef itk::DiscreteGaussianImageFilter< Mask, ITKHelpers::FloatScalarImageType >  BlurFilterType;
  BlurFilterType::Pointer gaussianFilter = BlurFilterType::New();
  gaussianFilter->SetInput(this->MaskImage);
  gaussianFilter->SetVariance(blurVariance);
  gaussianFilter->Update();

//   OutputHelpers::WriteImageConditional(gaussianFilter->GetOutput(),
//                                        "Debug/ComputeBoundaryNormals.BlurredMask.mha", this->DebugImages);

  // Compute the gradient of the blurred mask
  typedef itk::GradientImageFilter< ITKHelpers::FloatScalarImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

//   OutputHelpers::WriteImageConditional(gradientFilter->GetOutput(),
//                                        "Debug/ComputeBoundaryNormals.BlurredMaskGradient.mha", this->DebugImages);

  // Only keep the normals at the boundary
  typedef itk::MaskImageFilter< ITKHelpers::FloatVector2ImageType, ITKHelpers::UnsignedCharScalarImageType, ITKHelpers::FloatVector2ImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(gradientFilter->GetOutput());
  maskFilter->SetMaskImage(this->BoundaryImage);
  maskFilter->Update();

//   HelpersOutput::WriteImageConditional(maskFilter->GetOutput(),
//                                        "Debug/ComputeBoundaryNormals.BoundaryNormalsUnnormalized.mha",
//                                         this->DebugImages);

  // Allocate the image to return
  ITKHelpers::DeepCopy(maskFilter->GetOutput(), boundaryNormalsImage);

  // Normalize the vectors because we just care about their direction
  // (the Data term computation calls for the normalized boundary normal)
  itk::ImageRegionIterator<ITKHelpers::FloatVector2ImageType> boundaryNormalsIterator(
              boundaryNormalsImage, boundaryNormalsImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage,
                                           this->BoundaryImage->GetLargestPossibleRegion());

  while(!boundaryNormalsIterator.IsAtEnd())
    {
    if(boundaryIterator.Get()) // The pixel is on the boundary
      {
      ITKHelpers::FloatVector2ImageType::PixelType p = boundaryNormalsIterator.Get();
      p.Normalize();
      boundaryNormalsIterator.Set(p);
      }
    ++boundaryNormalsIterator;
    ++boundaryIterator;
    }
}
