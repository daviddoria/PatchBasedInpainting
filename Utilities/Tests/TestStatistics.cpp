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
#include "Statistics.h"
#include "Testing/Testing.h"

// STL
#include <stdexcept>

int main(int argc, char*argv[])
{
  // Average unsigned char
  {
  std::vector<unsigned char> v;

  for(unsigned int i = 1; i <= 6; ++i)
    {
    v.push_back(i);
    }

  //unsigned char av = Statistics::Average(v);
  float av = Statistics::Average(v);
  float correctAverage = 3.5;
  // std::cout << "av: " << static_cast<int>(av) << std::endl;

  if(!Testing::ValuesEqual(av, correctAverage))
    {
    std::stringstream ss;
    ss << "Unsigned char average test: average is " << static_cast<float>(av)
       << " but should be " << correctAverage << "!";
    throw std::runtime_error(ss.str());
    }
  }

  // Average float
  {
  std::vector<float> v;

  for(unsigned int i = 1; i <= 6; ++i)
    {
    v.push_back(i);
    }

  float correctAverage = 3.5;
  float av = Statistics::Average(v);
  //std::cout << "av: " << av << std::endl;

  if(!Testing::ValuesEqual(av, correctAverage))
    {
    std::stringstream ss;
    ss << "Float average test: average is " << av << " but should be " << correctAverage << "!";
    throw std::runtime_error(ss.str());
    }
  }
  
  // Running average unsigned char
  {
  std::vector<unsigned char> v;

  for(unsigned int i = 1; i <= 6; ++i)
    {
    v.push_back(i);
    }

  //std::cout << "av: " << static_cast<int>(av) << std::endl;
  float correctAverage = 3.5;
  //unsigned char runningAverage = Statistics::RunningAverage(v);
  float runningAverage = Statistics::RunningAverage(v);
  //std::cout << "runningAverage: " << static_cast<int>(runningAverage) << std::endl;

  if(!Testing::ValuesEqual(runningAverage, correctAverage))
    {
    std::stringstream ss;
    ss << "Unsigned char running average test: average is " << static_cast<float>(runningAverage)
       << " but should be " << correctAverage << "!";
    throw std::runtime_error(ss.str());
    }
  }

  // Running average float
  {
  std::vector<float> v;

  for(unsigned int i = 1; i <= 6; ++i)
    {
    v.push_back(i);
    }

  float correctAverage = 3.5;
  float runningAverage = Statistics::RunningAverage(v);
  //std::cout << "runningAverage: " << runningAverage << std::endl;

  if(!Testing::ValuesEqual(correctAverage, runningAverage))
    {
    std::stringstream ss;
    ss << "Float running average test: average is " << runningAverage << " but should be " << correctAverage << "!";
    throw std::runtime_error(ss.str());
    }
  }
  
  return EXIT_SUCCESS;
}
