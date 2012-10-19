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

#include "itkCovariantVector.h"

#include <iostream>
#include <vector>

#include "DifferenceFunctions/SumSquaredPixelDifference.hpp"

int main(int, char *[])
{
  typedef itk::CovariantVector<float, 3> VectorType;
  VectorType a;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;

  VectorType b;
  b[0] = 5;
  b[1] = 5;
  b[2] = 10;


  SumSquaredPixelDifference<VectorType> differenceFunction;
  float difference = differenceFunction(a,b);
  std::cout << "pixel difference: " << difference << std::endl;

  return EXIT_SUCCESS;
}

