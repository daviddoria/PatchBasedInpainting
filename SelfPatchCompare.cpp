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

SelfPatchCompare::SelfPatchCompare(const unsigned int components, CandidatePairs& candidatePairs) : Pairs(candidatePairs)
{
  this->NumberOfComponentsPerPixel = components;
}

void SelfPatchCompare::SetImage(const FloatVectorImageType::Pointer image)
{
  this->Image = image;
}

void SelfPatchCompare::SetMask(const Mask::Pointer mask)
{
  this->MaskImage = mask;
}

void SelfPatchCompare::ComputeOffsets()
{
  try
  {
    // Iterate over the target region of the mask. Add the linear offset of valid pixels to the offsets to be used later in the comparison.
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->Pairs.TargetPatch.Region);

    //unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    
    while(!maskIterator.IsAtEnd())
      {
      if(this->MaskImage->IsValid(maskIterator.GetIndex()))
	{
	//FloatVectorImageType::OffsetValueType offset = this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
      FloatVectorImageType::OffsetValueType offset = this->Image->ComputeOffset(maskIterator.GetIndex()) * this->NumberOfComponentsPerPixel;
	//std::cout << "Using offset: " << offset << std::endl;
	this->ValidOffsets.push_back(offset); // We have to multiply the linear offset by the number of components per pixel for the VectorImage type
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

float SelfPatchCompare::SlowDifference(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.
  
  // This method uses 3 iterators - one for the mask, and one for each image patch.
  // The entire mask is traversed looking for valid pixels, and then comparing the image pixels.
  // This is very inefficient because, since the target region stays constant for many thousands of patch
  // comparisons, the mask need only be traversed once. This method is performed by ComputeOffsets()
  // and PatchDifference*(). This function is only here for comparison purposes (to ensure the result of the other functions
  // is correct).
  
  try
  {
    //assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    itk::ImageRegion<2> newSourceRegion = sourcePatch.Region;
    itk::ImageRegion<2> newTargetRegion = this->Pairs.TargetPatch.Region;

    if(!this->Image->GetLargestPossibleRegion().IsInside(this->Pairs.TargetPatch.Region))
      {
      // Move the source region to the target region. We move this way because we want to iterate over the mask in the target region.
      itk::Offset<2> sourceTargetOffset = this->Pairs.TargetPatch.Region.GetIndex() - sourcePatch.Region.GetIndex();

      newSourceRegion.SetIndex(sourcePatch.Region.GetIndex() + sourceTargetOffset);

      // Force both regions to be entirely inside the image
      newTargetRegion.Crop(this->Image->GetLargestPossibleRegion());
      newSourceRegion.Crop(this->Image->GetLargestPossibleRegion());

      // Move the source region back to its original position
      newSourceRegion.SetIndex(newSourceRegion.GetIndex() - sourceTargetOffset);
      }

    //std::cout << "New source region: " << newSourceRegion << std::endl;
    //std::cout << "New target region: " << newTargetRegion << std::endl;
    itk::ImageRegionConstIterator<FloatVectorImageType> sourcePatchIterator(this->Image, newSourceRegion);
    itk::ImageRegionConstIterator<FloatVectorImageType> targetPatchIterator(this->Image, newTargetRegion);
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, newTargetRegion);

    float sum = 0;
    unsigned int validPixelCounter = 0;
    //unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    FullSquaredPixelDifference differenceFunction(this->Image->GetNumberOfComponentsPerPixel());
    
    while(!sourcePatchIterator.IsAtEnd())
      {
      itk::Index<2> currentPixel = maskIterator.GetIndex();
      if(this->MaskImage->IsValid(currentPixel))
        {
        //std::cout << "Offset from iterator: " << this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
        FloatVectorImageType::PixelType sourcePixel = sourcePatchIterator.Get();
        FloatVectorImageType::PixelType targetPixel = targetPatchIterator.Get();
        //std::cout << "Source pixel: " << sourcePixel << " target pixel: " << targetPixel << std::endl;
        //float difference = Helpers::PixelSquaredDifference(sourcePixel, targetPixel);
	float difference = differenceFunction.Difference(sourcePixel, targetPixel);
        sum +=  difference;
        validPixelCounter++;
        }

      ++sourcePatchIterator;
      ++targetPatchIterator;
      ++maskIterator;
      } // end while iterate over sourcePatch

    //std::cout << "totalDifference: " << sum << std::endl;
    //std::cout << "Valid pixels: " << validPixelCounter << std::endl;

    if(validPixelCounter == 0)
      {
      std::cerr << "Zero valid pixels in PatchDifference." << std::endl;
      std::cerr << "Source region: " << sourcePatch.Region << std::endl;
      std::cerr << "Target region: " << this->Pairs.TargetPatch.Region << std::endl;
      std::cerr << "New source region: " << newSourceRegion << std::endl;
      std::cerr << "New target region: " << newTargetRegion << std::endl;
      exit(-1);
      }
    float averageDifference = sum/static_cast<float>(validPixelCounter);
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

float SelfPatchCompare::PatchDifferenceManual(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    assert(this->Image->GetLargestPossibleRegion().IsInside(sourcePatch.Region));

    float totalDifference = 0;

    unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    
    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->Pairs.TargetPatch.Region.GetIndex())
                                    - this->Image->ComputeOffset(sourcePatch.Region.GetIndex())) * componentsPerPixel;

    float difference = 0;
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {
      /*
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
        targetPixel[i] = buffptr[this->ValidOffsets[pixelId] + i];
        sourcePixel[i] = buffptr[this->ValidOffsets[pixelId] - offsetDifference + i];
        }
      //std::cout << "Source pixel: " << sourcePixel << " target pixel: " << targetPixel << std::endl;
      float difference = Helpers::PixelSquaredDifference(sourcePixel, targetPixel);
      */
      difference = 0;
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
	//std::cout << "component " << i << ": " << buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i] << std::endl;
        //difference += fabs(buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
      
	difference += (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]) * 
		      (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
        }
      //std::cout << "difference: " << difference << std::endl;
      totalDifference += difference;
      }
    //std::cout << "totalDifference: " << totalDifference << std::endl;
    /*
    if(validPixelCounter == 0)
      {
      std::cerr << "Zero valid pixels in PatchDifference." << std::endl;
      std::cerr << "Source region: " << sourceRegion << std::endl;
      std::cerr << "Target region: " << targetRegion << std::endl;
      std::cerr << "New source region: " << newSourceRegion << std::endl;
      std::cerr << "New target region: " << newTargetRegion << std::endl;
      exit(-1);
      }
    */
    totalDifference *= totalDifference;
    float averageDifference = totalDifference/static_cast<float>(this->ValidOffsets.size());
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void SelfPatchCompare::SetPatchAverageSquaredDifference(PatchPair& patchPair)
{
  // Only compute if the values are not already computed.
  if(!patchPair.IsValidAverageSquaredDifference())
    {
    float averageSquaredDifference = PatchAverageSquaredDifference(patchPair.SourcePatch);
    patchPair.SetAverageSquaredDifference(averageSquaredDifference);
    }
}

void SelfPatchCompare::SetPatchColorDifference(PatchPair& patchPair)
{
  float colorDifference = PatchAverageDifference<ColorPixelDifference>(patchPair.SourcePatch);
  patchPair.SetColorDifference(colorDifference);
}

void SelfPatchCompare::SetPatchDepthDifference(PatchPair& patchPair)
{
  float depthDifference = PatchAverageDifference<DepthPixelDifference>(patchPair.SourcePatch);
  patchPair.SetDepthDifference(depthDifference);
}

float SelfPatchCompare::PatchAverageSquaredDifference(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    //assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    float totalDifference = 0.0f;
    
    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->Pairs.TargetPatch.Region.GetIndex())
                                    - this->Image->ComputeOffset(sourcePatch.Region.GetIndex())) * this->NumberOfComponentsPerPixel;

    float squaredDifference = 0;
    
    FloatVectorImageType::PixelType sourcePixel;
    sourcePixel.SetSize(this->NumberOfComponentsPerPixel);
    
    FloatVectorImageType::PixelType targetPixel;
    targetPixel.SetSize(this->NumberOfComponentsPerPixel);
    
    FloatVectorImageType::PixelType differencePixel;
    differencePixel.SetSize(this->NumberOfComponentsPerPixel);
    
    FullSquaredPixelDifference differenceFunction(sourcePixel);
    
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {
      
      for(unsigned int i = 0; i < this->NumberOfComponentsPerPixel; ++i)
        {
	sourcePixel[i] = buffptr[this->ValidOffsets[pixelId] + i];
        targetPixel[i] = buffptr[this->ValidOffsets[pixelId] - offsetDifference + i];
        }
    
      
      squaredDifference = differenceFunction.Difference(sourcePixel, targetPixel);
      //difference = NonVirtualPixelDifference(sourcePixel, targetPixel); // This call seems to make it very slow?
      //difference = (sourcePixel-targetPixel).GetSquaredNorm(); // horribly slow
      
      //differencePixel = sourcePixel-targetPixel;
      //difference = differencePixel.GetSquaredNorm();
      
//       difference = 0;
//       for(unsigned int i = 0; i < componentsPerPixel; ++i)
//         {
// 	difference += (sourcePixel[i] - targetPixel[i]) * 
// 		      (sourcePixel[i] - targetPixel[i]);
// 	}

      //totalDifference += difference;
      totalDifference += squaredDifference;
      }

    float averageDifference = totalDifference/static_cast<float>(this->ValidOffsets.size());
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference(PatchPair& patchPair)
{
  if(!patchPair.IsValidAverageAbsoluteDifference())
    {
    float averageAbsoluteDifference = PatchAverageAbsoluteSourceDifference(patchPair.SourcePatch);
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

float SelfPatchCompare::PatchAverageAbsoluteSourceDifference(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    //assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    float totalAbsoluteDifference = 0.0f;

    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->Pairs.TargetPatch.Region.GetIndex())
                                    - this->Image->ComputeOffset(sourcePatch.Region.GetIndex())) * this->NumberOfComponentsPerPixel;

    float absoluteDifference = 0;

    FloatVectorImageType::PixelType sourcePixel;
    sourcePixel.SetSize(this->NumberOfComponentsPerPixel);

    FloatVectorImageType::PixelType targetPixel;
    targetPixel.SetSize(this->NumberOfComponentsPerPixel);

    FloatVectorImageType::PixelType differencePixel;
    differencePixel.SetSize(this->NumberOfComponentsPerPixel);

    FullPixelDifference differenceFunction(this->NumberOfComponentsPerPixel);
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {

      for(unsigned int i = 0; i < this->NumberOfComponentsPerPixel; ++i)
        {
        sourcePixel[i] = buffptr[this->ValidOffsets[pixelId] + i];
        targetPixel[i] = buffptr[this->ValidOffsets[pixelId] - offsetDifference + i];
        }


      absoluteDifference = differenceFunction.Difference(sourcePixel, targetPixel);

      totalAbsoluteDifference += absoluteDifference;
      }
  
    float averageAbsoluteDifference = totalAbsoluteDifference / static_cast<float>(this->ValidOffsets.size());
    return averageAbsoluteDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


float SelfPatchCompare::PatchDifferenceBoundary(const Patch& sourcePatch)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    //assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    itk::ImageRegion<2> newSourceRegion = sourcePatch.Region;

    // Move the source region to the target region. We move this way because we want to iterate over the mask in the target region.
    itk::Offset<2> sourceTargetOffset = this->Pairs.TargetPatch.Region.GetIndex() - sourcePatch.Region.GetIndex();

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
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->Pairs.TargetPatch.Region.GetIndex())
                                    - this->Image->ComputeOffset(newSourceRegion.GetIndex())) * componentsPerPixel;

    float difference = 0;
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {
      difference = 0;
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
	//std::cout << "component " << i << ": " << buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i] << std::endl;
        //difference += fabs(buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
	difference += (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]) * 
		      (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
        }
      totalDifference += difference;
      }

    float averageDifference = totalDifference/static_cast<float>(this->ValidOffsets.size());
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
  SetPatchAverageSquaredDifference(patchPair);
  SetPatchAverageAbsoluteSourceDifference(patchPair);
  SetPatchColorDifference(patchPair);
  SetPatchDepthDifference(patchPair);
}

void SelfPatchCompare::ComputeAllSourceAndTargetDifferences()
{
  // Source patches are always full and entirely valid, so there are two cases - when the target patch is fully inside the image,
  // and when it is not.
  //std::cout << "ComputeAllSourceAndTargetDifferences()" << std::endl;
  try
  {
    // Force the target region to be entirely inside the image
    this->Pairs.TargetPatch.Region.Crop(this->Image->GetLargestPossibleRegion());
  
    ComputeOffsets();

    QtConcurrent::blockingMap<std::vector<PatchPair> >(this->Pairs, boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteFullDifference, this, _1));
    //std::cout << "Leaving ComputeAllSourceAndTargetDifferences()" << std::endl;
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
  // Source patches are always full and entirely valid, so there are two cases - when the target patch is fully inside the image,
  // and when it is not.
  try
  {
    //std::cout << "ComputeAllSourceDifferences()" << std::endl;
    // If the target region is not fully inside the image, crop it and proceed.
    if(!this->Image->GetLargestPossibleRegion().IsInside(this->Pairs.TargetPatch.Region))
      {  
      // Force the target region to be entirely inside the image
      this->Pairs.TargetPatch.Region.Crop(this->Image->GetLargestPossibleRegion());
    
      ComputeOffsets();
    
      for(unsigned int sourcePatchId = 0; sourcePatchId < this->Pairs.size(); ++sourcePatchId)
	{
	float distance = PatchDifferenceBoundary(this->Pairs[sourcePatchId].SourcePatch);
	this->Pairs[sourcePatchId].SetAverageAbsoluteDifference(distance);
	this->Pairs[sourcePatchId].SetAverageSquaredDifference(distance);
	}
      }
    else // The target patch is entirely inside the image
      {
      ComputeOffsets();
      /*
      for(unsigned int sourcePatchId = 0; sourcePatchId < this->Pairs.size(); ++sourcePatchId)
	{
	SetPatchAllDifferences(this->Pairs[sourcePatchId]);
	}
      */
      
      QtConcurrent::blockingMap<std::vector<PatchPair> >(this->Pairs, boost::bind(&SelfPatchCompare::SetPatchAllDifferences, this, _1));
      //QtConcurrent::blockingMap<std::vector<PatchPair> >(this->Pairs, boost::bind(&SelfPatchCompare::SetPatchTotalAbsoluteDifference, this, _1));
      //QtConcurrent::blockingMap<std::vector<PatchPair> >(this->Pairs, boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference, this, _1));
      }
    //std::cout << "Leaving ComputeAllSourceDifferences()" << std::endl;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllDifferences!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

void SelfPatchCompare::SetPairs(CandidatePairs& pairs)
{
  this->Pairs = pairs;
}
