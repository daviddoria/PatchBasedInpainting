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

#ifndef PriorityCriminisi_HPP
#define PriorityCriminisi_HPP

#include "PriorityCriminisi.h" // Make syntax parser happy

// Custom
#include "ImageProcessing/BoundaryNormals.h"
#include "ImageProcessing/Isophotes.h"

// Submodules
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>

// ITK
#include "itkDiscreteGaussianImageFilter.h"
#include "itkGradientImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkInvertIntensityImageFilter.h"

template <typename TImage>
PriorityCriminisi<TImage>::PriorityCriminisi(const TImage* const image, const Mask* const maskImage,
                                             const unsigned int patchRadius) :
  PriorityConfidence(maskImage, patchRadius), Image(image), BlurVariance(2.0f)
{
  this->BoundaryNormalsImage = Vector2ImageType::New();
  //ITKHelpers::InitializeImage(this->BoundaryNormalsImage.GetPointer(), image->GetLargestPossibleRegion());

  unsigned int blurVariance = 2;
  ComputeBoundaryNormals(blurVariance);

  this->IsophoteImage = Vector2ImageType::New();
  ITKHelpers::InitializeImage(this->IsophoteImage.GetPointer(), image->GetLargestPossibleRegion());
  Isophotes::ComputeColorIsophotesInRegion(image, maskImage, image->GetLargestPossibleRegion(),
                                           this->IsophoteImage.GetPointer());

}

template <typename TImage>
template <typename TNode>
void PriorityCriminisi<TImage>::Update(const TNode& sourceNode, const TNode& targetNode,
                                       const unsigned int patchNumber)
{
  Superclass::Update(sourceNode, targetNode, patchNumber);

  // Compute the isophotes we will need
//  itk::Index<2> targetIndex = Helpers::ConvertFrom<itk::Index<2>, TNode>(targetNode);
//  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(targetIndex, this->PatchRadius);
//  Isophotes::ComputeColorIsophotesInRegion(this->Image, this->MaskImage, region, this->IsophoteImage);

  // For debugging, we want to do this over the whole image
  Isophotes::ComputeColorIsophotesInRegion(this->Image, this->MaskImage,
                                           this->Image->GetLargestPossibleRegion(), this->IsophoteImage.GetPointer());

  if(this->IsDebugOn())
  {
    std::stringstream ss_isophote;
    ss_isophote << "IsophoteImage_" << patchNumber << ".mha";
    ITKHelpers::WriteSequentialImage(this->IsophoteImage.GetPointer(), "IsophoteImage", patchNumber, 3, "mha");
  }

  ComputeBoundaryNormals(this->BlurVariance);

  if(this->IsDebugOn())
  {
    ITKHelpers::WriteSequentialImage(this->BoundaryNormalsImage.GetPointer(), "BoundaryNormals", patchNumber, 3, "mha");

    WriteDataImage(Helpers::GetSequentialFileName("DataImage", patchNumber, "mha", 3));
  }

}

template <typename TImage>
template <typename TNode>
float PriorityCriminisi<TImage>::ComputePriority(const TNode& queryPixel) const
{
  //std::cout << "PriorityCriminisi::ComputePriority()" << std::endl;
  float confidenceTerm = ComputeConfidenceTerm(queryPixel);
  float dataTerm = ComputeDataTerm(queryPixel);

  float priority = confidenceTerm * dataTerm;

  return priority;
}


template <typename TImage>
float PriorityCriminisi<TImage>::ComputeDataTerm(const itk::Index<2>& queryPixel) const
{
  // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

  Vector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
  Vector2Type boundaryNormal = this->BoundaryNormalsImage->GetPixel(queryPixel);

  float dot = std::abs(isophote * boundaryNormal); // operator*() is the dot product

  // This doesn't actually contribue anything, since the argmax of the priority is all that is used,
  // and alpha ends up just being a scaling factor since the proiority is purely multiplicative.
  float alpha = 255;
  float dataTerm = dot/alpha;

  return dataTerm;
}

template <typename TImage>
void PriorityCriminisi<TImage>::ComputeBoundaryNormals(const float blurVariance)
{
  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  boundaryImage->SetRegions(this->Image->GetLargestPossibleRegion());
  boundaryImage->Allocate();

  this->MaskImage->FindBoundary(boundaryImage, Mask::VALID, 255);
  
  BoundaryNormals boundaryNormals(boundaryImage, this->MaskImage);

  boundaryNormals.ComputeBoundaryNormals(blurVariance, this->BoundaryNormalsImage.GetPointer());
}

template <typename TImage>
void PriorityCriminisi<TImage>::WriteDataImage(const std::string& fileName)
{
  // This is not used for the algorithm, but just for debugging output.
  FloatScalarImageType::Pointer dataImage = FloatScalarImageType::New();
  dataImage->SetRegions(this->Image->GetLargestPossibleRegion());
  dataImage->Allocate();

  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  boundaryImage->SetRegions(this->Image->GetLargestPossibleRegion());
  boundaryImage->Allocate();
  
  this->MaskImage->FindBoundary(boundaryImage, Mask::VALID, 255);

  typedef std::vector<itk::Index<2> > PixelCollection;
  PixelCollection boundaryPixels = ITKHelpers::GetNonZeroPixels(boundaryImage.GetPointer());

  for(PixelCollection::const_iterator iter = boundaryPixels.begin(); iter != boundaryPixels.end(); ++iter)
  {
    dataImage->SetPixel(*iter, ComputePriority(*iter));
  }

  ITKHelpers::WriteImage(dataImage.GetPointer(), fileName);
}

#endif
