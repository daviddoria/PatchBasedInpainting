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

#include "itkTimeProbe.h"
#include "itkVariableLengthVector.h"

#include "Helpers/ITKHelpers.h"

int main(int argc, char*argv[])
{
  unsigned int numberOfTrials = 1e7;

  itk::TimeProbe clock;

  clock.Start();

  itk::VariableLengthVector<float> v1;
  v1.SetSize(3);
  
  itk::VariableLengthVector<float> v2;
  v2.SetSize(3);

  for(unsigned int i = 0; i < 3; ++i)
  {
    v1[i] = drand48();
    v2[i] = drand48();
  }

  {
  float totalDifference = 0;
  for(unsigned int i = 0; i < numberOfTrials; ++i)
  {
    float difference = (v1-v2).GetNorm();
    totalDifference += difference;
  }

  std::cout << totalDifference << std::endl;
  clock.Stop();
  std::cout << "Mean: " << clock.GetMean() << std::endl;
  std::cout << "Total: " << clock.GetTotal() << std::endl;

  }

  clock.Start();

  {
  float totalDifference = 0;
  for(unsigned int i = 0; i < numberOfTrials; ++i)
  {
    float difference = ITKHelpers::SumOfComponents(v1-v2);
    totalDifference += difference;
  }
  std::cout << totalDifference << std::endl;
  clock.Stop();
  std::cout << "Mean: " << clock.GetMean() << std::endl;
  std::cout << "Total: " << clock.GetTotal() << std::endl;
  }

  
  return EXIT_SUCCESS;
}
