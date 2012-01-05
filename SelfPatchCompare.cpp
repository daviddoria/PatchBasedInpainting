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

SelfPatchCompare::SelfPatchCompare() : Image(NULL), MaskImage(NULL)
{

}

void SelfPatchCompare::SetImage(const FloatVectorImageType* image)
{
  //std::cout << "Enter SelfPatchCompare::SetImage()" << std::endl;
  this->Image = image;
  this->NumberOfComponentsPerPixel = image->GetNumberOfComponentsPerPixel();
  //std::cout << "Leave SelfPatchCompare::SetImage()" << std::endl;
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
    this->ValidTargetPatchPixelOffsets.clear();

    // Iterate over the target region of the mask. Add the linear offset of valid pixels to the offsets to be used later in the comparison.
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->Pairs->GetTargetPatch().GetRegion());
    itk::Index<2> targetCorner = this->Pairs->GetTargetPatch().GetIndex();
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
        // The ComputeOffset function returns the linear index of the pixel.
        // To compute the memory address of the pixel, we must multiply by the number of components per pixel.
        itk::Offset<2> offset = maskIterator.GetIndex() - targetCorner;
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
  float colorDifference = PatchAverageSourceDifference<PixelDifferenceColor>(patchPair.GetSourcePatch());
  patchPair.GetDifferences().SetDifferenceByType(PairDifferences::ColorDifference, colorDifference);
}

void SelfPatchCompare::SetPatchDepthDifference(PatchPair& patchPair)
{
  float depthDifference = PatchAverageSourceDifference<PixelDifferenceDepth>(patchPair.GetSourcePatch());
  patchPair.GetDifferences().SetDifferenceByType(PairDifferences::DepthDifference, depthDifference);
}


void SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference(PatchPair& patchPair)
{
  float averageAbsoluteDifference = PatchAverageSourceDifference<PixelDifferenceFull>(patchPair.GetSourcePatch());
  patchPair.GetDifferences().SetDifferenceByType(PairDifferences::AverageAbsoluteDifference, averageAbsoluteDifference);
}

void SelfPatchCompare::ComputeAllSourceDifferences()
{
  // EnterFunction("SelfPatchCompare::ComputeAllSourceDifferences()");
  // Source patches are always full and entirely valid, so there are two cases - when the target patch is fully inside the image,
  // and when it is not.
  ComputeOffsets();
  //std::cout << "Enter SelfPatchCompare::ComputeAllSourceDifferences parallel SetPatchAllDifferences" << std::endl;
  std::cout << "SelfPatchCompare::ComputeAllSourceDifferences had: " << this->ValidTargetPatchOffsets.size()
            << " ValidTargetPatchOffsets on which to operate!" << std::endl;
  #ifdef USE_QT_PARALLEL
    #pragma message("Using QtConcurrent!")
    QtConcurrent::blockingMap(this->Pairs->begin(), this->Pairs->end(), boost::bind(&SelfPatchCompare::SetPatchAllDifferences, this, _1));
  #else
    #pragma message("NOT using QtConcurrent!")
    for(CandidatePairs::Iterator pairIterator = this->Pairs->begin(); pairIterator != this->Pairs->end(); ++pairIterator)
      {
      SetPatchAllDifferences(*pairIterator);
      }
  #endif
}

void SelfPatchCompare::SetPairs(CandidatePairs* const pairs)
{
  //std::cout << "Enter SelfPatchCompare::SetPairs()" << std::endl;
  this->Pairs = pairs;
  //std::cout << "Leave SelfPatchCompare::SetPairs()" << std::endl;
}

void SelfPatchCompare::SetNumberOfComponentsPerPixel(const unsigned int numberOfComponentsPerPixel)
{
  this->NumberOfComponentsPerPixel = numberOfComponentsPerPixel;
}
