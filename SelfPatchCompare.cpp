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

#include "SelfPatchCompare.h"

// Custom
#include "Helpers.h"
#include "Patch.h"
#include "PatchPair.h"
#include "PixelDifference.h"
#include "Types.h"

// Qt
#include <QtConcurrentMap>

// Boost
#include <boost/bind.hpp>

SelfPatchCompare::SelfPatchCompare()
{
  this->NumberOfComponentsPerPixel = 1;
}

void SelfPatchCompare::SetImage(const FloatVectorImageType::Pointer image)
{
  //std::cout << "Enter SelfPatchCompare::SetImage()" << std::endl;
  this->Image = image;
  this->NumberOfComponentsPerPixel = image->GetNumberOfComponentsPerPixel();
  //std::cout << "Leave SelfPatchCompare::SetImage()" << std::endl;
}

void SelfPatchCompare::SetMask(const Mask::Pointer mask)
{
  //std::cout << "Enter SelfPatchCompare::SetMask()" << std::endl;
  this->MaskImage = mask;
  //std::cout << "Leave SelfPatchCompare::SetMask()" << std::endl;
}

void SelfPatchCompare::ComputeOffsets()
{
  // This function computes the list of offsets that are from the source region of the target patch.
  try
  {
    //std::cout << "Computing offsets for TargetPatch: " << this->Pairs->TargetPatch.Region.GetIndex() << std::endl;
    this->ValidTargetPatchOffsets.clear();
    
    if(this->Image->GetNumberOfComponentsPerPixel() != this->NumberOfComponentsPerPixel)
      {
      std::cerr << "this->Image->GetNumberOfComponentsPerPixel() does not match this->NumberOfComponentsPerPixel!" << std::endl;
      exit(-1);
      }
    // Iterate over the target region of the mask. Add the linear offset of valid pixels to the offsets to be used later in the comparison.
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->Pairs->TargetPatch.Region);
    
    while(!maskIterator.IsAtEnd())
      {
      if(this->MaskImage->IsValid(maskIterator.GetIndex()))
	{
	//FloatVectorImageType::OffsetValueType offset = this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
	if(!this->Image->GetLargestPossibleRegion().IsInside(maskIterator.GetIndex()))
	  {
	  std::cerr << "SelfPatchCompare::ComputeOffsets - Something is wrong!" << std::endl;
	  exit(-1);
	  }
	
	FloatVectorImageType::OffsetValueType offset = this->Image->ComputeOffset(maskIterator.GetIndex()) * this->NumberOfComponentsPerPixel;
	if(offset < 0)
	  {
	  std::cerr << "SelfPatchCompare::ComputeOffsets - offset is negative!" << std::endl;
	  exit(-1);
	  }
	//std::cout << "Using offset: " << offset << std::endl;
	this->ValidTargetPatchOffsets.push_back(offset); // We have to multiply the linear offset by the number of components per pixel for the VectorImage type
	}

      ++maskIterator;
      }
    //std::cout << "Number of valid offsets: " << this->ValidOffsets.size() << std::endl;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeOffsets!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
/*
void SelfPatchCompare::SetPatchAverageSquaredDifference(PatchPair& patchPair)
{
  // Only compute if the values are not already computed.
//   if(!patchPair.IsValidAverageSquaredDifference())
//     {
//     float averageSquaredDifference = PatchAverageSquaredDifference(patchPair.SourcePatch);
//     patchPair.SetAverageSquaredDifference(averageSquaredDifference);
//     }
  float averageSquaredDifference = PatchAverageSquaredDifference(patchPair.SourcePatch);
  patchPair.SetAverageSquaredDifference(averageSquaredDifference);
}
*/
void SelfPatchCompare::SetPatchColorDifference(PatchPair& patchPair)
{
  float colorDifference = PatchAverageSourceDifference<ColorPixelDifference>(patchPair.SourcePatch);
  patchPair.SetColorDifference(colorDifference);
}

void SelfPatchCompare::SetPatchDepthDifference(PatchPair& patchPair)
{
  float depthDifference = PatchAverageSourceDifference<DepthPixelDifference>(patchPair.SourcePatch);
  patchPair.SetDepthDifference(depthDifference);
}


void SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference(PatchPair& patchPair)
{
  if(!patchPair.IsValidAverageAbsoluteDifference())
    {
    float averageAbsoluteDifference = PatchAverageSourceDifference<FullPixelDifference>(patchPair.SourcePatch);
    patchPair.SetAverageAbsoluteDifference(averageAbsoluteDifference);
    }
}

void SelfPatchCompare::SetPatchAverageAbsoluteFullDifference(PatchPair& patchPair)
{
  // Don't recompute.
  if(patchPair.IsValidAverageAbsoluteDifference())
    {
    return;
    }
  itk::ImageRegionConstIterator<FloatVectorImageType> sourcePatchIterator(this->Image, patchPair.SourcePatch.Region);
  itk::ImageRegionConstIterator<FloatVectorImageType> targetPatchIterator(this->Image, patchPair.TargetPatch.Region);
  
  float totalAbsoluteDifference = 0.0f;

  FullPixelDifference differenceFunction(this->Image->GetNumberOfComponentsPerPixel());
  while(!sourcePatchIterator.IsAtEnd())
    {
    totalAbsoluteDifference += differenceFunction.Difference(sourcePatchIterator.Get(), targetPatchIterator.Get());

    ++sourcePatchIterator;
    ++targetPatchIterator;
    }
    
  float averageAbsoluteDifference = totalAbsoluteDifference / static_cast<float>(patchPair.SourcePatch.Region.GetNumberOfPixels());
  patchPair.SetAverageAbsoluteDifference(averageAbsoluteDifference);
}

float SelfPatchCompare::PatchSourceDifferenceBoundary(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    //assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    itk::ImageRegion<2> newSourceRegion = sourcePatch.Region;

    // Move the source region to the target region. We move this way because we want to iterate over the mask in the target region.
    itk::Offset<2> sourceTargetOffset = this->Pairs->TargetPatch.Region.GetIndex() - sourcePatch.Region.GetIndex();

    newSourceRegion.SetIndex(sourcePatch.Region.GetIndex() + sourceTargetOffset);

    // Force the source region to be entirely inside the image
    newSourceRegion.Crop(this->Image->GetLargestPossibleRegion());

    // Move the source region back to its original position
    newSourceRegion.SetIndex(newSourceRegion.GetIndex() - sourceTargetOffset);

    //std::cout << "New source region: " << newSourceRegion << std::endl;
    //std::cout << "New target region: " << newTargetRegion << std::endl;
    
    float totalDifference = 0;

    unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    
    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->Pairs->TargetPatch.Region.GetIndex())
                                    - this->Image->ComputeOffset(newSourceRegion.GetIndex())) * componentsPerPixel;

    float difference = 0;
    for(unsigned int pixelId = 0; pixelId < this->ValidTargetPatchOffsets.size(); ++pixelId)
      {
      difference = 0;
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
	//std::cout << "component " << i << ": " << buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i] << std::endl;
        //difference += fabs(buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
	difference += (buffptr[this->ValidTargetPatchOffsets[pixelId] + i] - buffptr[this->ValidTargetPatchOffsets[pixelId] - offsetDifference + i]) * 
		      (buffptr[this->ValidTargetPatchOffsets[pixelId] + i] - buffptr[this->ValidTargetPatchOffsets[pixelId] - offsetDifference + i]);
        }
      totalDifference += difference;
      }

    float averageDifference = totalDifference/static_cast<float>(this->ValidTargetPatchOffsets.size());
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void SelfPatchCompare::SetPatchAllDifferences(PatchPair& patchPair)
{
  //std::cout << "Enter SelfPatchCompare::SetPatchAllDifferences()" << std::endl;
  //SetPatchAverageSquaredDifference(patchPair);
  try
  {
    for(unsigned int i = 0; i < this->FunctionsToCompute.size(); ++i)
      {
      this->FunctionsToCompute[i](patchPair);
      }
    //std::cout << "Leave SelfPatchCompare::SetPatchAllDifferences()" << std::endl;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in SetPatchAllDifferences!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void SelfPatchCompare::ComputeAllSourceAndTargetDifferences()
{
  //std::cout << "Enter SelfPatchCompare::ComputeAllSourceAndTargetDifferences()" << std::endl;
  // Source patches are always full and entirely valid, so there are two cases - when the target patch is fully inside the image,
  // and when it is not.
  //std::cout << "ComputeAllSourceAndTargetDifferences()" << std::endl;
  try
  {
    // Force the target region to be entirely inside the image
    this->Pairs->TargetPatch.Region.Crop(this->Image->GetLargestPossibleRegion());
  
    ComputeOffsets();

    QtConcurrent::blockingMap<std::vector<PatchPair> >(*(this->Pairs), boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteFullDifference, this, _1));
    //std::cout << "Leave SelfPatchCompare::ComputeAllSourceAndTargetDifferences()" << std::endl;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllSourceAndTargetDifferences!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void SelfPatchCompare::ComputeAllSourceDifferences()
{
  //std::cout << "Enter SelfPatchCompare::ComputeAllSourceDifferences()" << std::endl;
  // Source patches are always full and entirely valid, so there are two cases - when the target patch is fully inside the image,
  // and when it is not.
  try
  {
    //std::cout << "ComputeAllSourceDifferences()" << std::endl;
    // If the target region is not fully inside the image, crop it and proceed.
    if(!this->Image->GetLargestPossibleRegion().IsInside(this->Pairs->TargetPatch.Region))
      {  
      // Force the target region to be entirely inside the image
      this->Pairs->TargetPatch.Region.Crop(this->Image->GetLargestPossibleRegion());
    
      ComputeOffsets();
    
      for(unsigned int sourcePatchId = 0; sourcePatchId < this->Pairs->size(); ++sourcePatchId)
	{
	float distance = PatchSourceDifferenceBoundary((*this->Pairs)[sourcePatchId].SourcePatch);
	(*this->Pairs)[sourcePatchId].SetAverageAbsoluteDifference(distance);
	(*this->Pairs)[sourcePatchId].SetAverageSquaredDifference(distance);
	}
      }
    else // The target patch is entirely inside the image
      {
      ComputeOffsets();
      //std::cout << "Enter SelfPatchCompare::ComputeAllSourceDifferences parallel SetPatchAllDifferences" << std::endl;
      
      // Parallel version
      //QtConcurrent::blockingMap<std::vector<PatchPair> >((*this->Pairs), boost::bind(&SelfPatchCompare::SetPatchAllDifferences, this, _1));
    
      // Serial version
      for(unsigned int i = 0; i < this->Pairs->size(); ++i)
	{
	SetPatchAllDifferences((*this->Pairs)[i]);
	}
      }
    //std::cout << "Leave SelfPatchCompare::ComputeAllSourceDifferences()" << std::endl;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllDifferences!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void SelfPatchCompare::SetPairs(CandidatePairs* pairs)
{
  //std::cout << "Enter SelfPatchCompare::SetPairs()" << std::endl;
  this->Pairs = pairs;
  //std::cout << "Leave SelfPatchCompare::SetPairs()" << std::endl;
}

void SelfPatchCompare::SetNumberOfComponentsPerPixel(const unsigned int numberOfComponentsPerPixel)
{
  this->NumberOfComponentsPerPixel = numberOfComponentsPerPixel;
}
