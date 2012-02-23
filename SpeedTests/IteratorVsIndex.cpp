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

#include <vector>

#include "Helpers/ITKHelpers.h"

int main(int argc, char*argv[])
{
  itk::TimeProbe clock;

  clock.Start();

  std::vector<float> vec;

  for(unsigned int i = 0; i < 1e7; ++i)
  {
    vec.push_back(drand48());
  }

  {
  float total = 0;
  for(unsigned int i = 0; i < vec.size(); ++i)
  {
    total += vec[i];
  }

  std::cout << total << std::endl;
  clock.Stop();
  std::cout << "Mean: " << clock.GetMean() << std::endl;
  std::cout << "Total: " << clock.GetTotal() << std::endl;

  }

  clock.Start();

  {
  float total = 0;
  for(std::vector<float>::const_iterator iter = vec.begin(); iter != vec.end(); ++iter)
  {
    total += *iter;
  }
  std::cout << total << std::endl;
  clock.Stop();
  std::cout << "Mean: " << clock.GetMean() << std::endl;
  std::cout << "Total: " << clock.GetTotal() << std::endl;
  }


  return EXIT_SUCCESS;
}
