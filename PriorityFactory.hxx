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

#include "PriorityFactory.h" // to appease syntax parser

#include "Helpers/Helpers.h"
#include "PriorityCriminisi.h"
#include "PriorityDepth.h"
#include "PriorityManual.h"
#include "PriorityOnionPeel.h"
#include "PriorityRandom.h"
#include "Types.h"

template <typename TImage>
typename PriorityFactory<TImage>::PriorityNameMapType PriorityFactory<TImage>::CreateNameMap()
{
  PriorityNameMapType nameMap;
  nameMap[MANUAL] = "Manual";
  nameMap[RANDOM] = "Random";
  nameMap[CRIMINISI] = "Criminisi";
  nameMap[ONIONPEEL] = "OnionPeel";
  nameMap[DEPTH] = "Depth";
  return nameMap;
}

template <typename TImage>
typename PriorityFactory<TImage>::PriorityNameMapType PriorityFactory<TImage>::DifferenceNameMap = CreateNameMap();

template <typename TImage>
typename PriorityFactory<TImage>::PriorityTypes PriorityFactory<TImage>::PriorityTypeFromName(const std::string& nameOfPriority)
{
  typename PriorityNameMapType::const_iterator iterator;
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

template <typename TImage>
std::string PriorityFactory<TImage>::NameOfPriority(const PriorityTypes typeOfPriority)
{
  typename PriorityNameMapType::const_iterator iterator;

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

template <typename TImage>
Priority<TImage>* PriorityFactory<TImage>::Create(const PriorityTypes priorityType, const TImage* const image,
                                  const Mask* const maskImage, const unsigned int patchRadius)
{
  if(priorityType == MANUAL)
    {
    return new PriorityManual<TImage, PriorityOnionPeel>(image, maskImage, patchRadius);
    }
  else if(priorityType == ONIONPEEL)
    {
    return new PriorityOnionPeel<TImage>(image, maskImage, patchRadius);
    }
  else if(priorityType == RANDOM)
    {
    return new PriorityRandom<TImage>(image, maskImage, patchRadius);
    }
  else if(priorityType == DEPTH)
    {
    return new PriorityDepth<TImage>(image, maskImage, patchRadius);
    }
  else if(priorityType == CRIMINISI)
    {
    return new PriorityCriminisi<TImage>(image, maskImage, patchRadius);
    }
  else
    {
    std::cerr << "Priority type " << priorityType << " is unknown!" << std::endl;
    exit(-1);
    return NULL;
    }
}

template <typename TImage>
std::vector<std::string> PriorityFactory<TImage>::GetImageNames(const PriorityFactory::PriorityTypes& priorityType)
{
  std::vector<std::string> priorityImageNames;

  if(priorityType == MANUAL)
    {
    priorityImageNames = PriorityManual<TImage, PriorityOnionPeel>::GetImageNames();
    }
  else if(priorityType == ONIONPEEL)
    {
    priorityImageNames = PriorityOnionPeel<TImage>::GetImageNames();
    }
  else if(priorityType == RANDOM)
    {
    priorityImageNames = PriorityRandom<TImage>::GetImageNames();
    }
  else if(priorityType == DEPTH)
    {
    priorityImageNames = PriorityDepth<TImage>::GetImageNames();
    }
  else if(priorityType == CRIMINISI)
    {
    priorityImageNames = PriorityCriminisi<TImage>::GetImageNames();
    }

  return priorityImageNames;
}
