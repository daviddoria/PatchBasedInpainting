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

#include "PriorityFactory.h"

#include "Helpers/Helpers.h"
#include "PriorityCriminisi.h"
#include "PriorityDepth.h"
#include "PriorityManual.h"
#include "PriorityOnionPeel.h"
#include "PriorityRandom.h"
#include "Types.h"


PriorityFactory::PriorityNameMapType PriorityFactory::CreateNameMap()
{
  PriorityNameMapType nameMap;
  nameMap[MANUAL] = "Manual";
  nameMap[RANDOM] = "Random";
  nameMap[CRIMINISI] = "Criminisi";
  nameMap[ONIONPEEL] = "OnionPeel";
  nameMap[DEPTH] = "Depth";
  return nameMap;
}

PriorityFactory::PriorityNameMapType PriorityFactory::DifferenceNameMap = CreateNameMap();

PriorityFactory::PriorityTypes PriorityFactory::PriorityTypeFromName(const std::string& nameOfPriority)
{
  PriorityNameMapType::const_iterator iterator;
  for (iterator = DifferenceNameMap.begin(); iterator != DifferenceNameMap.end(); ++iterator)
    {
    if (iterator->second == nameOfPriority)
      {
      return iterator->first;
      break;
      }
    }
  return INVALID;
}

std::string PriorityFactory::NameOfPriority(const PriorityTypes typeOfPriority)
{
  PriorityNameMapType::const_iterator iterator;

  iterator = DifferenceNameMap.find(typeOfPriority);
  if(iterator != DifferenceNameMap.end())
    {
    return iterator->second;
    }
  else
    {
    return "Invalid";
    }
}

Priority* PriorityFactory::Create(const PriorityTypes priorityType, const FloatVectorImageType* const image,
                                  const Mask* const maskImage, const unsigned int patchRadius)
{
  if(priorityType == MANUAL)
    {
    return new PriorityManual<PriorityOnionPeel>(image, maskImage, patchRadius);
    }
  else if(priorityType == ONIONPEEL)
    {
    return new PriorityOnionPeel(image, maskImage, patchRadius);
    }
  else if(priorityType == RANDOM)
    {
    return new PriorityRandom(image, maskImage, patchRadius);
    }
  else if(priorityType == DEPTH)
    {
    return new PriorityDepth(image, maskImage, patchRadius);
    }
  else if(priorityType == CRIMINISI)
    {
    return new PriorityCriminisi(image, maskImage, patchRadius);
    }
  else
    {
    std::cerr << "Priority type " << priorityType << " is unknown!" << std::endl;
    exit(-1);
    return NULL;
    }
}

std::vector<std::string> PriorityFactory::GetImageNames(const PriorityFactory::PriorityTypes& priorityType)
{
  std::vector<std::string> priorityImageNames;

  if(priorityType == MANUAL)
    {
    priorityImageNames = PriorityManual<PriorityOnionPeel>::GetImageNames();
    }
  else if(priorityType == ONIONPEEL)
    {
    priorityImageNames = PriorityOnionPeel::GetImageNames();
    }
  else if(priorityType == RANDOM)
    {
    priorityImageNames = PriorityRandom::GetImageNames();
    }
  else if(priorityType == DEPTH)
    {
    priorityImageNames = PriorityDepth::GetImageNames();
    }
  else if(priorityType == CRIMINISI)
    {
    priorityImageNames = PriorityCriminisi::GetImageNames();
    }

  return priorityImageNames;
}
