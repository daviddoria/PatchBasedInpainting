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

#include "CandidatePairs.h" // Appease syntax parser


template <typename TImage>
void CandidatePairs::VisitAllPatchPairs(const TImage* const image, const Mask* const mask, PatchPairVisitor &visitor)
{
//   #ifdef USE_QT_PARALLEL
//     #error Parallel version not yet implemented
//     #pragma message("Using QtConcurrent!")
//     QVector<float> differences = QtConcurrent::blockingMapped<QVector<float> >(this->begin(), this->end(),
//                                                                                boost::bind(&PixelPairVisitor::VisitOffsets, _1, _2, _3));
//   #else
//     #pragma message("NOT using QtConcurrent!")
// 
//     std::vector<itk::Offset<2> > offsets = ComputeOffsets(mask);
// 
//     for(ConstIterator patchIterator = this->begin(); patchIterator != this->end(); ++patchIterator)
//       {
//       (*patchIterator).VisitOffsets(image, offsets, visitor);
//       }
//   #endif
  
  std::vector<itk::Offset<2> > offsets = ComputeOffsets(mask);

  for(ConstIterator patchIterator = this->begin(); patchIterator != this->end(); ++patchIterator)
    {
    //(*patchIterator).VisitOffsets(image, offsets, visitor);
    visitor.Visit(*patchIterator);
    }
}

