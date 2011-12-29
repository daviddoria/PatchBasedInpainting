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
#include "ClusterColorsAdaptive.h"
#include "Helpers.h"
#include "Histograms.h"
#include "Patch.h"
#include "PatchPair.h"
#include "PixelDifference.h"
#include "Types.h"

// Qt
#ifdef USE_QT_PARALLEL
  #include <QtConcurrentMap>
#endif

// Boost
#include <boost/bind.hpp>

// SelfPatchCompare::SelfPatchCompare(const FloatVectorImageType* image, const Mask* mask) :
// ColorFrequency(NULL), Image(NULL), MembershipImage(NULL), MaskImage(NULL), NumberOfComponentsPerPixel(1), Image(image), Mask(mask)
// {
//   this->NumberOfComponentsPerPixel = image->GetNumberOfComponentsPerPixel();
// }

SelfPatchCompare::SelfPatchCompare() :
Image(NULL), MembershipImage(NULL), MaskImage(NULL), NumberOfComponentsPerPixel(1)
{

}

void SelfPatchCompare::SetImage(const FloatVectorImageType* image)
{
  //std::cout << "Enter SelfPatchCompare::SetImage()" << std::endl;
  this->Image = image;
  this->NumberOfComponentsPerPixel = image->GetNumberOfComponentsPerPixel();
  //std::cout << "Leave SelfPatchCompare::SetImage()" << std::endl;
}

// Provide the membership image (used in some difference functions).
void SelfPatchCompare::SetMembershipImage(IntImageType* const membershipImage)
{
  this->MembershipImage = membershipImage;
}

void SelfPatchCompare::SetMask(const Mask* mask)
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
	// The ComputeOffset function returns the linear index of the pixel. To compute the memory address of the pixel, we must multiply by the number of components per pixel.
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
    std::cout << "Number of valid offsets: " << this->ValidTargetPatchOffsets.size() << std::endl;
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
  patchPair.DifferenceMap[PatchPair::ColorDifference] = colorDifference;
}
/*
void SelfPatchCompare::SetPatchMembershipDifference(PatchPair& patchPair)
{
  if(!this->MembershipImage)
    {
    std::cerr << "No membership image set!" << std::endl;
    exit(-1);
    }
  float membershipDifference = PatchAverageSourceDifference<IntImageType, ScalarPixelDifference<IntImageType::PixelType> >(this->MembershipImage, patchPair.SourcePatch);
  //float membershipDifference = PatchAverageSourceDifference<IntImageType, ScalarAllOrNothingPixelDifference<IntImageType::PixelType> >(this->MembershipImage, patchPair.SourcePatch);
  patchPair.DifferenceMap[PatchPair::MembershipDifference] = membershipDifference;
}
*/

void SelfPatchCompare::SetPatchHistogramIntersection(PatchPair& patchPair)
{
//   if(!this->ColorFrequency)
//     {
//     std::cerr << "No ClusterColors/ColorFrequency set!" << std::endl;
//     exit(-1);
//     }
// //   std::vector<float> histogram1 = this->ColorFrequency->HistogramRegion(this->ColorBinMembershipImage,
// //                                                                         bestPatchPair.TargetPatch.Region, this->MaskImage, bestPatchPair.TargetPatch.Region);
// //   std::vector<float> histogram2 = this->ColorFrequency->HistogramRegion(this->ColorBinMembershipImage,
// //                                                                         bestPatchPair.SourcePatch.Region, inverseMask, bestPatchPair.TargetPatch.Region);
//   std::vector<float> histogram1 = this->ColorFrequency->HistogramRegion(this->Image, patchPair.TargetPatch.Region, this->MaskImage, patchPair.TargetPatch.Region);
//    std::vector<float> histogram2 = this->ColorFrequency->HistogramRegion(this->Image, patchPair.SourcePatch.Region,
//                                                                          this->MaskImage, patchPair.TargetPatch.Region, true);
// 
//   float histogramIntersection = Histograms::HistogramIntersection(histogram2, histogram1);
//   patchPair.DifferenceMap[PatchPair::HistogramIntersection] = histogramIntersection;
}

void SelfPatchCompare::SetPatchDepthDifference(PatchPair& patchPair)
{
  float depthDifference = PatchAverageSourceDifference<DepthPixelDifference>(patchPair.SourcePatch);
  patchPair.DifferenceMap[PatchPair::DepthDifference] = depthDifference;
}


void SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference(PatchPair& patchPair)
{
  float averageAbsoluteDifference = PatchAverageSourceDifference<FullPixelDifference>(patchPair.SourcePatch);
  patchPair.DifferenceMap[PatchPair::AverageAbsoluteDifference] = averageAbsoluteDifference;
}

void SelfPatchCompare::SetPatchAverageAbsoluteFullDifference(PatchPair& patchPair)
{
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
  patchPair.DifferenceMap[PatchPair::AverageAbsoluteDifference] = averageAbsoluteDifference;
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
    #ifdef USE_QT_PARALLEL
      #pragma message("Using QtConcurrent!")
      QtConcurrent::blockingMap<std::vector<PatchPair> >(*(this->Pairs), boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteFullDifference, this, _1));
    #else
      #pragma message("NOT using QtConcurrent!")
      for(unsigned int i = 0; i < this->Pairs->size(); ++i)
        {
        SetPatchAverageAbsoluteFullDifference((*this->Pairs)[i]);
        }
    #endif
    //LeaveFunction("SelfPatchCompare::ComputeAllSourceAndTargetDifferences()");
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
  // EnterFunction("SelfPatchCompare::ComputeAllSourceDifferences()");
  // Source patches are always full and entirely valid, so there are two cases - when the target patch is fully inside the image,
  // and when it is not.
  try
  {

    ComputeOffsets();
    //std::cout << "Enter SelfPatchCompare::ComputeAllSourceDifferences parallel SetPatchAllDifferences" << std::endl;
    std::cout << "SelfPatchCompare::ComputeAllSourceDifferences had: " << this->ValidTargetPatchOffsets.size() << " ValidTargetPatchOffsets on which to operate!" << std::endl;
    #ifdef USE_QT_PARALLEL
      #pragma message("Using QtConcurrent!")
      QtConcurrent::blockingMap<std::vector<PatchPair> >((*this->Pairs), boost::bind(&SelfPatchCompare::SetPatchAllDifferences, this, _1));
    #else
      #pragma message("NOT using QtConcurrent!")
      for(unsigned int i = 0; i < this->Pairs->size(); ++i)
        {
        SetPatchAllDifferences((*this->Pairs)[i]);
        }
    #endif

    // Serial only - for testing
//       for(unsigned int i = 0; i < this->Pairs->size(); ++i)
//         {
//         SetPatchAllDifferences((*this->Pairs)[i]);
//         }

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

const float SelfPatchCompare::MaxColorDifference = 255*255;
