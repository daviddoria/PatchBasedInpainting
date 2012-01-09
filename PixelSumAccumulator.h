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

#ifndef PixelSumAccumulator_H
#define PixelSumAccumulator_H

#include "PixelVisitor.h"

template <typename TPixel>
class PixelSumAccumulator : public PixelVisitor<TPixel>
{
public:

  PixelSumAccumulator() : Sum(0) {}
  void Visit(const TPixel &pixel)
  {
    this->Sum += pixel;
  }

  TPixel GetSum()
  {
    return this->Sum;
  }

  void Clear()
  {
    //this->Sum = TPixel::Zero;
    this->Sum = 0;
  }

private:

  TPixel Sum;
};

#endif
