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

#ifndef PRIORITYMANUAL_H
#define PRIORITYMANUAL_H

#include "Priority.h"
#include "PriorityConfidence.h"
#include "PriorityRandom.h"
#include "PriorityDepth.h"

/**
\class PriorityManual
\brief This class first returns values that have been specified by a user generated image
       that contains pixels that indicate "definitely fill first". Once there are no more of
       these pixels, the superclass's ComputePriority function is called.
*/
// template< class TImage, template<class> class TPriority>
// class PriorityManual : public TPriority<TImage>
template< typename TNode, typename TManualImage, typename TPriority>
class PriorityManual
{
public:
  typedef TNode NodeType;
  
  // Reimplemented functions
  PriorityManual(const TManualImage* const manualPriorityImage,
                 TPriority* const priorityFunction);

  float ComputePriority(const TNode& queryPixel) const;

  // New functions
  void SetManualPriorityImage(const TManualImage* const);

  void Update(const TNode& filledPixel);

  //void SetPriorityFunction(Priority* const priorityFunction);

protected:
  typename TManualImage::Pointer ManualPriorityImage;

  const TPriority* PriorityFunction;
};

#include "PriorityManual.hpp"

#endif
