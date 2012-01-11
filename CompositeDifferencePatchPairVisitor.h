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

#ifndef CompositeDifferencePatchPairVisitor_H
#define CompositeDifferencePatchPairVisitor_H

#include "PatchPairVisitor.h"

#include <cmath>

template <typename TPixel>
class CompositeDifferencePatchPairVisitor : public PatchPairVisitor<TPixel>
{
public:

  CompositeDifferencePatchVisitor() : Sum(0) {}

  void Visit(const PatchPair& patchPair)
  {
    for(unsigned int i = 0; i < this->Items.size(); ++i)
      {
      this->Visitors[i].Visit(patchPair);
      }
  }

private:

  std::vector<PatchPairVisitor*> Visitors;
};

#endif
