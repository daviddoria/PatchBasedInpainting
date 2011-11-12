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

#include "PriorityCriminisi.h"

// Custom
#include "Helpers.h"

// VXL
#include <vnl/vnl_double_2.h>

// ITK
#include "itkInvertIntensityImageFilter.h"

PriorityCriminisi::PriorityCriminisi(FloatVectorImageType::Pointer image, Mask::Pointer maskImage, unsigned int patchRadius) : Priority(image, maskImage, patchRadius)
{
  this->ConfidenceImage = FloatScalarImageType::New();
  this->ConfidenceMapImage = FloatScalarImageType::New();
  this->DataImage = FloatScalarImageType::New();
}
  
float PriorityCriminisi::ComputePriority(const itk::Index<2>& queryPixel)
{
  //double confidence = ComputeConfidenceTerm(queryPixel);
  //double data = ComputeDataTerm(queryPixel);

  float confidence = this->ConfidenceImage->GetPixel(queryPixel);
  float data = this->DataImage->GetPixel(queryPixel);

  float priority = confidence * data;

  return priority;
}


float PriorityCriminisi::ComputeDataTerm(const itk::Index<2>& queryPixel)
{
  // The difference between this funciton and Criminisi's original data term computation (ComputeDataTermCriminisi)
  // is that we claim there is no reason to penalize the priority of linear structures that don't have a perpendicular incident
  // angle with the boundary. Of course, we don't want to continue structures that are almost parallel with the boundary, but above
  // a threshold, the strength of the isophote should be more important than the angle of incidence.
  try
  {
    FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
    FloatVector2Type boundaryNormal = this->BoundaryNormalsImage->GetPixel(queryPixel);

    DebugMessage<FloatVector2Type>("Isophote: ", isophote);
    DebugMessage<FloatVector2Type>("Boundary normal: ", boundaryNormal);
    // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

    vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

    vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

    float dataTerm = 0.0f;

    float angleBetween = Helpers::AngleBetween(isophote, boundaryNormal);
    if(angleBetween < 20)
      {
      float projectionMagnitude = isophote.GetNorm() * cos(angleBetween);
      
      dataTerm = projectionMagnitude;
      }
    else
    {
      dataTerm = isophote.GetNorm();
    }

    return dataTerm;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

float PriorityCriminisi::ComputeDataTermCriminisi(const itk::Index<2>& queryPixel)
{
  try
  {
    FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
    FloatVector2Type boundaryNormal = this->BoundaryNormalsImage->GetPixel(queryPixel);

    DebugMessage<FloatVector2Type>("Isophote: ", isophote);
    DebugMessage<FloatVector2Type>("Boundary normal: ", boundaryNormal);
    // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

    vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

    vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

    float dot = std::abs(dot_product(vnlIsophote,vnlNormal));

    float alpha = 255; // This doesn't actually contribue anything, since the argmax of the priority is all that is used, and alpha ends up just being a scaling factor since the proiority is purely multiplicative.
    float dataTerm = dot/alpha;

    return dataTerm;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeDataTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void PriorityCriminisi::ComputeAllDataTerms()
{
  try
  {
    itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    // Blank the data term image
    this->DataImage->FillBuffer(0);

    while(!boundaryIterator.IsAtEnd())
      {
      if(boundaryIterator.Get() != 0) // This is a pixel on the current boundary
	{
	itk::Index<2> currentPixel = boundaryIterator.GetIndex();
	float dataTerm = ComputeDataTerm(currentPixel);
	this->DataImage->SetPixel(currentPixel, dataTerm);
	//DebugMessage<float>("Set DataTerm to ", dataTerm);
	}

      ++boundaryIterator;
      }
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllDataTerms!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void PriorityCriminisi::ComputeAllConfidenceTerms()
{
  try
  {
    itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    // Blank the data term image
    this->ConfidenceImage->FillBuffer(0);

    while(!boundaryIterator.IsAtEnd())
      {
      if(boundaryIterator.Get() != 0) // This is a pixel on the current boundary
	{
	itk::Index<2> currentPixel = boundaryIterator.GetIndex();
	float confidenceTerm = ComputeConfidenceTerm(currentPixel);
	this->ConfidenceImage->SetPixel(currentPixel, confidenceTerm);
	}

      ++boundaryIterator;
      }
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllConfidenceTerms!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void PriorityCriminisi::UpdateConfidences(const itk::ImageRegion<2>& targetRegion, const float value)
{
  DebugMessage("UpdateConfidences()");
  try
  {
    // Force the region to update to be entirely inside the image
    itk::ImageRegion<2> region = Helpers::CropToRegion(targetRegion, this->Image->GetLargestPossibleRegion());
    
    // Use an iterator to find masked pixels. Compute their new value, and save it in a vector of pixels and their new values.
    // Do not update the pixels until after all new values have been computed, because we want to use the old values in all of
    // the computations.
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

    while(!maskIterator.IsAtEnd())
      {
      if(this->MaskImage->IsHole(maskIterator.GetIndex()))
	{
	itk::Index<2> currentPixel = maskIterator.GetIndex();
	this->ConfidenceMapImage->SetPixel(currentPixel, value);
	}

      ++maskIterator;
      } // end while looop with iterator

  } // end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in UpdateConfidences!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void PriorityCriminisi::InitializeConfidenceMap()
{
  EnterFunction("InitializeConfidenceMap()");
  // Clone the mask - we need to invert the mask to actually perform the masking, but we don't want to disturb the original mask
  Mask::Pointer maskClone = Mask::New();
  //Helpers::DeepCopy<Mask>(this->CurrentMask, maskClone);
  maskClone->DeepCopyFrom(this->MaskImage);
  
  // Invert the mask
  typedef itk::InvertIntensityImageFilter <Mask> InvertIntensityImageFilterType;
  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(maskClone);
  //invertIntensityFilter->InPlaceOn();
  invertIntensityFilter->Update();

  // Convert the inverted mask to floats and scale them to between 0 and 1
  // to serve as the initial confidence image
  typedef itk::RescaleIntensityImageFilter< Mask, FloatScalarImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(invertIntensityFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();

  Helpers::DeepCopy<FloatScalarImageType>(rescaleFilter->GetOutput(), this->ConfidenceMapImage);
  LeaveFunction("InitializeConfidenceMap()");
}


float PriorityCriminisi::ComputeConfidenceTerm(const itk::Index<2>& queryPixel)
{
  //DebugMessage<itk::Index<2>>("Computing confidence for ", queryPixel);
  try
  {
    // Allow for regions on/near the image border

    //itk::ImageRegion<2> region = this->CurrentMask->GetLargestPossibleRegion();
    //region.Crop(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
    itk::ImageRegion<2> targetRegion = Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);
    itk::ImageRegion<2> region = Helpers::CropToRegion(targetRegion, this->Image->GetLargestPossibleRegion());
    
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

    // The confidence is computed as the sum of the confidences of patch pixels in the source region / area of the patch

    float sum = 0;

    while(!maskIterator.IsAtEnd())
      {
      if(this->MaskImage->IsValid(maskIterator.GetIndex()))
        {
        sum += this->ConfidenceMapImage->GetPixel(maskIterator.GetIndex());
        }
      ++maskIterator;
      }

    unsigned int numberOfPixels = region.GetNumberOfPixels();
    float areaOfPatch = static_cast<float>(numberOfPixels);
    
    float confidence = sum/areaOfPatch;
    DebugMessage<float>("Confidence: ", confidence);

    return confidence;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeConfidenceTerm!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
