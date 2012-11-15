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

#include "FeatureVectorPixelDescriptor.h"

#include <algorithm>

FeatureVectorPixelDescriptor::FeatureVectorPixelDescriptor(const FeatureVectorType& featureVector) :
  FeatureVector(featureVector)
{

}

FeatureVectorPixelDescriptor::FeatureVectorPixelDescriptor(const unsigned int length)
{
  this->FeatureVector.resize(length);
  std::fill(this->FeatureVector.begin(), this->FeatureVector.end(), 0);
}

const FeatureVectorPixelDescriptor::FeatureVectorType& FeatureVectorPixelDescriptor::GetFeatureVector() const
{
  return this->FeatureVector;
}

std::ostream& operator<<(std::ostream& output, const std::vector<float>& descriptor)
{
  for(unsigned int i = 0; i < descriptor.size(); ++i)
  {
    output << descriptor[i] << " ";
  }
  //output << std::endl;
  
  return output;
}

std::ostream& operator<<(std::ostream& output, const FeatureVectorPixelDescriptor& descriptor)
{
  if(descriptor.GetStatus() == PixelDescriptor::INVALID)
  {
    //output << "invalid";
  }
  else if(descriptor.GetStatus() == PixelDescriptor::TARGET_NODE)
  {
//     output << "Descriptor size: " << descriptor.GetFeatureVector().size() << std::endl;
//     output << "Status: target" << std::endl;;
//     output << std::endl << "Descriptor: ";
//     output << descriptor.GetFeatureVector() << std::endl;
  }
  else if(descriptor.GetStatus() == PixelDescriptor::SOURCE_NODE)
  {
    output << "Descriptor size: " << descriptor.GetFeatureVector().size() << std::endl;
    output << "Status: source" << std::endl;;
    output << std::endl << "Descriptor: ";
    output << descriptor.GetFeatureVector() << std::endl;
  }

  return output;
}
