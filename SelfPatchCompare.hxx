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

#ifdef IN_KDEVELOP_PARSER
#include "SelfPatchCompare.h"
#endif

// Custom
#include "CandidatePairs.h"
#include "Helpers/Helpers.h"
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

template <typename TImage>
SelfPatchCompare<TImage>::SelfPatchCompare() :
Pairs(NULL), Image(NULL), MaskImage(NULL)
{

}

template <typename TImage>
void SelfPatchCompare<TImage>::SetImage(const TImage* const image)
{
  this->Image = image;
}

template <typename TImage>
void SelfPatchCompare<TImage>::SetMask(const Mask* const mask)
{
  this->MaskImage = mask;
}

template <typename TImage>
void SelfPatchCompare<TImage>::ComputeOffsets()
{
  // This function computes the list of offsets that are from the source region of the target patch.
  try
  {
    //std::cout << "Computing offsets for TargetPatch: " << this->Pairs->TargetPatch.Region.GetIndex() << std::endl;
    this->ValidTargetPatchPixelOffsets.clear();

    // Iterate over the target region of the mask. Add the linear offset of valid pixels to the offsets to be used later in the comparison.
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->Pairs->GetTargetPatch().GetRegion());
    itk::Index<2> targetCorner = this->Pairs->GetTargetPatch().GetCorner();
    while(!maskIterator.IsAtEnd())
      {
      if(this->MaskImage->IsValid(maskIterator.GetIndex()))
        {
        // The ComputeOffset function returns the linear index of the pixel.
        // To compute the memory address of the pixel, we must multiply by the number of components per pixel.
        itk::Offset<2> offset = maskIterator.GetIndex() - targetCorner;

        //std::cout << "Using offset: " << offset << std::endl;
        this->ValidTargetPatchPixelOffsets.push_back(offset); // We have to multiply the linear offset by the number of components per pixel for the VectorImage type
        }

      ++maskIterator;
      }
    std::cout << "Number of valid offsets: " << this->ValidTargetPatchPixelOffsets.size() << std::endl;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeOffsets!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <typename TImage>
template<typename TPatchDifference>
void SelfPatchCompare<TImage>::Compute()
{
  assert(this->Image);
  assert(this->MaskImage);
  assert(this->MaskImage->GetLargestPossibleRegion() == this->Image->GetLargestPossibleRegion());
  assert(this->Pairs);

  ComputeOffsets();
  //std::cout << "Enter SelfPatchCompare::ComputeAllSourceDifferences parallel SetPatchAllDifferences" << std::endl;
  std::cout << "SelfPatchCompare::Compute() had: " << this->ValidTargetPatchPixelOffsets.size()
            << " ValidTargetPatchOffsets on which to operate!" << std::endl;

  TPatchDifference patchDifferenceFunction;
  for(unsigned int differenceTypeId = 0; differenceTypeId < this->DifferenceTypes.size(); ++differenceTypeId)
    {
    PairDifferences::PatchDifferenceTypes currentDifferenceType = this->DifferenceTypes[differenceTypeId];
    if(this->ValidTargetPatchPixelOffsets.size() == 0)
      {
      throw std::runtime_error("SelfPatchCompare::Compute() had no ValidTargetPatchOffsets!");
      }
    #ifdef USE_QT_PARALLEL
      #pragma message("Using QtConcurrent!")
      QVector<float> differences = QtConcurrent::blockingMapped<QVector<float> >(this->Pairs->begin(), this->Pairs->end(),
                                                                              boost::bind(&TPatchDifference::Difference, &patchDifferenceFunction, _1));

      unsigned int differenceId = 0;
      for(CandidatePairs::Iterator pairIterator = this->Pairs->begin(); pairIterator != this->Pairs->end(); ++pairIterator)
        {
        (*pairIterator).GetDifferences().SetDifferenceByType(currentDifferenceType, differences[differenceId]);
        }
    #else
      #pragma message("NOT using QtConcurrent!")

      patchDifferenceFunction.SetImage(this->Image);

      for(CandidatePairs::Iterator pairIterator = this->Pairs->begin(); pairIterator != this->Pairs->end(); ++pairIterator)
        {
        float difference = patchDifferenceFunction.Difference(*pairIterator, this->ValidTargetPatchPixelOffsets);
        (*pairIterator).GetDifferences().SetDifferenceByType(currentDifferenceType, difference);
        }
    #endif
    }
}

template <typename TImage>
void SelfPatchCompare<TImage>::SetPairs(CandidatePairs* const pairs)
{
  this->Pairs = pairs;
}

template <typename TImage>
void SelfPatchCompare<TImage>::SetDifferenceType(const PairDifferences::PatchDifferenceTypes& differenceType)
{
  this->DifferenceTypes.clear();
  this->DifferenceTypes.push_back(differenceType);
}

template <typename TImage>
void SelfPatchCompare<TImage>::SetDifferenceTypes(const std::vector<PairDifferences::PatchDifferenceTypes>& differenceTypes)
{
  this->DifferenceTypes = differenceTypes;
}

template <typename TImage>
void SelfPatchCompare<TImage>::AddDifferenceType(const PairDifferences::PatchDifferenceTypes& differenceType)
{
  this->DifferenceTypes.push_back(differenceType);
}
