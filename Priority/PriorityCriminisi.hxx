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

#include "PriorityCriminisi.h" // Make syntax parser happy

// Custom
#include "../ImageProcessing/BoundaryNormals.h"
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "../ImageProcessing/Isophotes.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"

// ITK
#include "itkDiscreteGaussianImageFilter.h"
#include "itkGradientImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkInvertIntensityImageFilter.h"

// VTK
#include <vtkSmartPointer.h>

template <typename TImage>
PriorityCriminisi<TImage>::PriorityCriminisi(const TImage* const image, const Mask* const maskImage, unsigned int patchRadius) :
PriorityOnionPeel(maskImage, patchRadius), Image(image)
{
  this->BoundaryNormalsImage = FloatVector2ImageType::New();
  ITKHelpers::InitializeImage<FloatVector2ImageType>(this->BoundaryNormalsImage, image->GetLargestPossibleRegion());

  unsigned int blurVariance = 2;
  ComputeBoundaryNormals(blurVariance);

  this->IsophoteImage = FloatVector2ImageType::New();
  ITKHelpers::InitializeImage<FloatVector2ImageType>(this->IsophoteImage, image->GetLargestPossibleRegion());
  Isophotes::ComputeColorIsophotesInRegion(image, maskImage, image->GetLargestPossibleRegion(), this->IsophoteImage);

}

// template <typename TImage>
// std::vector<std::string> PriorityCriminisi<TImage>::GetImageNames()
// {
//   std::vector<std::string> imageNames = PriorityOnionPeel<TImage>::GetImageNames();
//   imageNames.push_back("Isophotes");
//   imageNames.push_back("BoundaryNormals");
//   return imageNames;
// }
// 
// template <typename TImage>
// std::vector<NamedVTKImage> PriorityCriminisi<TImage>::GetNamedImages()
// {
//   std::vector<NamedVTKImage> namedImages = PriorityOnionPeel<TImage>::GetNamedImages();
// 
//   NamedVTKImage isophoteNamedImage;
//   isophoteNamedImage.Name = "Isophotes";
//   vtkSmartPointer<vtkImageData> isophoteImageVTK = vtkSmartPointer<vtkImageData>::New();
//   ITKVTKHelpers::ITKImageToVTKVectorFieldImage(this->IsophoteImage, isophoteImageVTK);
//   isophoteNamedImage.ImageData = isophoteImageVTK;
//   isophoteNamedImage.DisplayType = NamedVTKImage::VECTORS;
//   namedImages.push_back(isophoteNamedImage);
// 
//   NamedVTKImage boundaryNormalsNamedImage;
//   boundaryNormalsNamedImage.Name = "BoundaryNormals";
//   vtkSmartPointer<vtkImageData> boundaryNormalsImageVTK = vtkSmartPointer<vtkImageData>::New();
//   ITKVTKHelpers::ITKImageToVTKVectorFieldImage(this->BoundaryNormalsImage, boundaryNormalsImageVTK);
//   boundaryNormalsNamedImage.ImageData = isophoteImageVTK;
//   boundaryNormalsNamedImage.DisplayType = NamedVTKImage::VECTORS;
//   namedImages.push_back(boundaryNormalsNamedImage);
// 
//   return namedImages;
// }

template <typename TImage>
void PriorityCriminisi<TImage>::Update(const itk::Index<2>& filledPixel)
{
  Superclass::Update(filledPixel);

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(filledPixel, this->PatchRadius);
  Isophotes::ComputeColorIsophotesInRegion(this->Image, this->MaskImage, region, this->IsophoteImage);

  unsigned int blurVariance = 2;
  ComputeBoundaryNormals(blurVariance);
}

template <typename TImage>
float PriorityCriminisi<TImage>::ComputePriority(const itk::Index<2>& queryPixel) const
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

  FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
  FloatVector2Type boundaryNormal = this->BoundaryNormalsImage->GetPixel(queryPixel);

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
  // TODO: The boundary image is no longer stored, so should we just blur the mask image locally
  // to compute the boundary normal for each pixel separately (per ComputePriority() call)?
  // BoundaryNormals boundaryNormals(this->BoundaryImage, this->MaskImage);
  // this->BoundaryNormalsImage = boundaryNormals.ComputeBoundaryNormals(blurVariance);

  //HelpersOutput::WriteImageConditional<FloatVector2ImageType>(this->BoundaryNormalsImage, "Debug/ComputeBoundaryNormals.BoundaryNormals.mha", this->DebugImages);
}

template <typename TImage>
FloatScalarImageType* PriorityCriminisi<TImage>::GetDataImage()
{
  // TODO: Actually create the data image. This is not used for the algorithm, but just for debugging output.
  FloatScalarImageType::Pointer dataImage = FloatScalarImageType::New();
  return dataImage;
}
