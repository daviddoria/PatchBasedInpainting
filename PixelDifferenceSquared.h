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

#ifndef PixelDifferenceSquared_H
#define PixelDifferenceSquared_H

#include "PixelDifference.h"

template <typename TPixel>
struct PixelDifferenceSquared
{
public:
  PixelDifferenceSquared(const TPixel& examplePixel)
  {
    this->NumberOfComponentsPerPixel = examplePixel.GetNumberOfElements();
  }

  PixelDifferenceSquared(const unsigned int numberOfComponents)
  {
    this->NumberOfComponentsPerPixel = numberOfComponents;
    //std::cout << "FullSquaredPixelDifference set NumberOfComponentsPerPixel to " << this->NumberOfComponentsPerPixel << std::endl;
  }

  float Difference(const TPixel& a, const TPixel& b)
  {
    return Difference(a, b, this->NumberOfComponentsPerPixel);
  }

  static float Difference(const TPixel& a, const TPixel& b, const unsigned int numberOfComponents)
  {
    float difference = PixelDifference<TPixel>::Difference(a, b, numberOfComponents);
    return difference*difference;
  }

private:
  unsigned int NumberOfComponentsPerPixel;
};

#endif
