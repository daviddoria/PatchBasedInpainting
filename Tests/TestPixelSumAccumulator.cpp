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

#include "PixelSumAccumulator.h"
#include "Types.h"
#include "Testing.h"

static void TestScalar();

int main(int argc, char*argv[])
{
  TestScalar();

  return EXIT_SUCCESS;
}

void TestScalar()
{
  std::vector<int> pixels;
  pixels.push_back(1);
  pixels.push_back(2);
  pixels.push_back(3);

  PixelSumAccumulator<int> pixelSumAccumulator;

  for(unsigned int i = 0; i < pixels.size(); ++i)
    {
    pixelSumAccumulator.Visit(pixels[i]);
    }

  int pixelSum = pixelSumAccumulator.GetSum();
  int expectedSum = 6;
  if(pixelSum != expectedSum)
    {
    std::stringstream ss;
    ss << "Scalar: sum should be " << expectedSum << " but it is " << pixelSum;
    throw std::runtime_error(ss.str());
    }

}
