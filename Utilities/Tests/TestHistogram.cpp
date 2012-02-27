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
#include "Helpers/OutputHelpers.h"
#include "Histogram.h"
#include "Testing/Testing.h"

// STL
#include <stdexcept>

int main(int argc, char*argv[])
{
  std::vector<float> values;
  values.push_back(1);
  values.push_back(1);
  values.push_back(1);
  values.push_back(5);
  
  std::vector<float> histogram = Histogram::ScalarHistogram(values, 2, 0.0f, 10.0f);
  OutputHelpers::OutputVector(histogram);
  
  return EXIT_SUCCESS;
}