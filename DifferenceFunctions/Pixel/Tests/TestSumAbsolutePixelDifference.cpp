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

#include "SumAbsolutePixelDifference.hpp"

//#include "Testing.h"

int main(int, char*[])
{
  typedef itk::CovariantVector<float, 3> PixelType;

  PixelType pixelA;
  pixelA.Fill(1);

  PixelType pixelB;
  pixelB.Fill(1);

  SumAbsolutePixelDifference<PixelType> sad;

  float difference = sad(pixelA, pixelB);

  std::cout << "sad: " << difference << std::endl;

  return EXIT_SUCCESS;
}
