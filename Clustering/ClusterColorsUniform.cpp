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

#include "ClusterColorsUniform.h"

void ClusterColorsUniform::SetNumberOfBinsPerAxis(const unsigned int numberOfBinsPerAxis)
{
  this->NumberOfBinsPerAxis = numberOfBinsPerAxis;
}


void ClusterColorsUniform::GenerateColors()
{
  EnterFunction("GenerateUniformColors");
  this->Colors.clear();
  unsigned int step = 256/this->NumberOfBinsPerAxis;
  ColorMeasurementVectorType color;
  for(unsigned int r = 0; r < 255; r += step)
    {
    for(unsigned int g = 0; g < 255; g += step)
      {
      for(unsigned int b = 0; b < 255; b += step)
        {
        color[0] = r;
        color[1] = g;
        color[2] = b;
        this->Colors.push_back(color);
        }
      }
    }

  CreateKDTreeFromColors();

  LeaveFunction("GenerateUniformColors");
}
