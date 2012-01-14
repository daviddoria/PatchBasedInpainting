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

#include "PointCloudDescriptorCreator.h" // Appease syntax parser

// Custom
#include "DescriptorItem.h"

// ITK
#include "itkIndex.h"

// STL
#include <fstream>
#include <stdexcept>

PointCloudDescriptorCreator::PointCloudDescriptorCreator(const std::string& fileName)
{
  std::ifstream fin(fileName.c_str());

  if(fin == NULL)
    {
    std::stringstream ss;
    ss << "Cannot open file " << fileName;
    throw std::runtime_error(ss.str());
    }

  std::string line;
  // TODO: Write this parser
//   while(getline(fin, line))
//     {
//     std::stringstream ss;
//     ss << line;
//     itk::Index<2> index;
//     ss >> index;
//   
//     std::vector<float> descriptor;
//     ss >> descriptor;
// 
//     Descriptors[index] = descriptor;
//     }

  fin.close();
}

Item* PointCloudDescriptorCreator::CreateItem(const itk::Index<2>& index) const
{
  //return new PointCloudDescriptorCreator(this->Descriptors[index]);

  DescriptorContainer::const_iterator iter = this->Descriptors.find(index);

  if(iter == this->Descriptors.end())
    {
    std::stringstream ss;
    ss << "Index " << index << " not found.";
    throw std::runtime_error(ss.str());
    }
  else
    {
    return new DescriptorItem(iter->second);
    //std::cout << "Found key " << iter->first << ". Associated value: " << iter->second << std::endl;
    }

}
