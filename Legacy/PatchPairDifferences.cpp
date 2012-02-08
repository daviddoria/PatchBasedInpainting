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

#include "PatchPairDifferences.h"

#include <cassert>

unsigned int PatchPairDifferences::GetNumberOfDifferences() const
{
  return this->DifferenceMap.size();
}

PatchPairDifferences::DifferenceNameMapType PatchPairDifferences::CreateNameMap()
{
  DifferenceNameMapType nameMap;
  nameMap[SumPixelDifference] = "SumPixelDifference";
  nameMap[AveragePixelDifference] = "AveragePixelDifference";
  nameMap[ColorDifference] = "ColorDifference";
  nameMap[DepthDifference] = "DepthDifference";
  return nameMap;
}

PatchPairDifferences::DifferenceNameMapType PatchPairDifferences::DifferenceNameMap = CreateNameMap();

void PatchPairDifferences::SetDifferenceByName(const std::string& differenceName, const float value)
{
  //this->DifferenceMap.find(TypeOfDifference(differenceName))->second = value;
  this->DifferenceMap[TypeOfDifference(differenceName)] = value;
}

void PatchPairDifferences::SetDifferenceByType(const PatchPairDifferenceTypes differenceType, const float value)
{
  //this->DifferenceMap.find(differenceType)->second = value;
  this->DifferenceMap[differenceType] = value;
}

float PatchPairDifferences::GetDifferenceByName(const std::string& nameOfDifference) const
{
  return this->DifferenceMap.find(TypeOfDifference(nameOfDifference))->second;
}

float PatchPairDifferences::GetDifferenceByType(const PatchPairDifferenceTypes differenceType) const
{
  return this->DifferenceMap.find(differenceType)->second;
}

std::vector<std::string> PatchPairDifferences::GetDifferenceNames() const
{
  std::vector<std::string> names;
  for(DifferenceMapType::const_iterator differenceIterator = this->DifferenceMap.begin(); differenceIterator != this->DifferenceMap.end(); ++differenceIterator)
    {
    names.push_back(NameOfDifference(differenceIterator->first));
    }
  return names;
}

void PatchPairDifferences::Invalidate()
{
  this->DifferenceMap.clear();
}

PatchPairDifferences::PatchPairDifferenceTypes PatchPairDifferences::TypeOfDifference(const std::string& nameOfDifference)
{
  DifferenceNameMapType::const_iterator iterator;
  for (iterator = DifferenceNameMap.begin(); iterator != DifferenceNameMap.end(); ++iterator)
    {
    if (iterator->second == nameOfDifference)
      {
      return iterator->first;
      break;
      }
    }
  return Invalid;
}

std::string PatchPairDifferences::NameOfDifference(const PatchPairDifferenceTypes typeOfDifference)
{
  DifferenceNameMapType::const_iterator iterator;

  iterator = DifferenceNameMap.find(typeOfDifference);
  if(iterator != DifferenceNameMap.end())
    {
    return iterator->second;
    }
  else
    {
    return "Invalid";
    }
}
