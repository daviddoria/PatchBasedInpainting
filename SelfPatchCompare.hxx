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


// Custom
#include "CandidatePairs.h"
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

template <typename TImage, typename TPatchDifference>
SelfPatchCompare<TImage, TPatchDifference>::SelfPatchCompare() : Image(NULL), MaskImage(NULL), DifferenceType(PairDifferences::Invalid)
{

}

template <typename TImage, typename TPatchDifference>
void SelfPatchCompare<TImage, TPatchDifference>::SetImage(const TImage* const image)
{
  this->Image = image;
}

template <typename TImage, typename TPatchDifference>
void SelfPatchCompare<TImage, TPatchDifference>::SetMask(const Mask* const mask)
{
  this->MaskImage = mask;
}

template <typename TImage, typename TPatchDifference>
void SelfPatchCompare<TImage, TPatchDifference>::ComputeOffsets()
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
        if(!this->Image->GetLargestPossibleRegion().IsInside(maskIterator.GetIndex()))
          {
          std::cerr << "SelfPatchCompare::ComputeOffsets - Something is wrong!" << std::endl;
          exit(-1);
          }
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

template <typename TImage, typename TPatchDifference>
void SelfPatchCompare<TImage, TPatchDifference>::Compute()
{
  // EnterFunction("SelfPatchCompare::ComputeAllSourceDifferences()");
  // Source patches are always full and entirely valid, so there are two cases - when the target patch is fully inside the image,
  // and when it is not.
  ComputeOffsets();
  //std::cout << "Enter SelfPatchCompare::ComputeAllSourceDifferences parallel SetPatchAllDifferences" << std::endl;
  std::cout << "SelfPatchCompare::Comput() had: " << this->ValidTargetPatchPixelOffsets.size()
            << " ValidTargetPatchOffsets on which to operate!" << std::endl;
  #ifdef USE_QT_PARALLEL
    #pragma message("Using QtConcurrent!")
    QVector<float> differences = QtConcurrent::blockingMap(this->Pairs->begin(), this->Pairs->end(), boost::bind(&TPatchDifference::Difference, _1));
    unsigned int differenceId = 0;
    for(CandidatePairs::Iterator pairIterator = this->Pairs->begin(); pairIterator != this->Pairs->end(); ++pairIterator)
      {
      (*pairIterator).GetDifferences().SetDifferenceByType(this->DifferenceType, differences[differenceId]);
      }
  #else
    #pragma message("NOT using QtConcurrent!")
    for(CandidatePairs::Iterator pairIterator = this->Pairs->begin(); pairIterator != this->Pairs->end(); ++pairIterator)
      {
      (*pairIterator).GetDifferences().SetDifferenceByType(this->DifferenceType, TPatchDifference::Difference(*pairIterator));
      }
  #endif
}

template <typename TImage, typename TPatchDifference>
void SelfPatchCompare<TImage, TPatchDifference>::SetPairs(CandidatePairs* const pairs)
{
  this->Pairs = pairs;
}

template <typename TImage, typename TPatchDifference>
void SelfPatchCompare<TImage, TPatchDifference>::SetDifferenceType(const PairDifferences::PatchDifferenceTypes& differenceType)
{
  this->DifferenceType = differenceType;
}
