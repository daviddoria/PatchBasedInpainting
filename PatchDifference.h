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

#ifndef PatchDifference_H
#define PatchDifference_H

#include <vector>

#include "itkOffset.h"

// Custom
class Mask;
#include "PatchPair.h"
#include "Types.h"

/**
\class PatchDifference
\brief This class is used to compute the difference between two patches.
*/
template <typename TImage, typename TPixelDifference>
class PatchDifference
{
public:
  /** Default constructor.*/
  PatchDifference();

  /** Compute the difference between two patches.*/
  float Difference(const PatchPair<TImage>& patchPair) const;

  /** Compute the difference between two patches only at specified offsets.*/
  virtual float Difference(const PatchPair<TImage>& patchPair, const std::vector<itk::Offset<2> >& offsetsToCompare) const = 0;

  /** Provide the image to work with.*/
  void SetImage(const TImage* const image);

protected:
  const TImage* Image;
};

#include "PatchDifference.hxx"

#endif
