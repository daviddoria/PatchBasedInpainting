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

#include "PairDifferences.h"

#include <cassert>

unsigned int PairDifferences::GetNumberOfDifferences() const
{
  return this->DifferenceMap.size();
}

PairDifferences::DifferenceNameMapType PairDifferences::CreateNameMap()
{
  DifferenceNameMapType nameMap;
  nameMap[SumPixelDifference] = "SumPixelDifference";
  nameMap[AveragePixelDifference] = "AveragePixelDifference";
  nameMap[ColorDifference] = "ColorDifference";
  nameMap[DepthDifference] = "DepthDifference";
  return nameMap;
}

PairDifferences::DifferenceNameMapType PairDifferences::DifferenceNameMap = CreateNameMap();

void PairDifferences::SetDifferenceByName(const std::string& differenceName, const float value)
{
  //this->DifferenceMap.find(TypeOfDifference(differenceName))->second = value;
  this->DifferenceMap[TypeOfDifference(differenceName)] = value;
}

void PairDifferences::SetDifferenceByType(const PatchDifferenceTypes differenceType, const float value)
{
  //this->DifferenceMap.find(differenceType)->second = value;
  this->DifferenceMap[differenceType] = value;
}

float PairDifferences::GetDifferenceByName(const std::string& nameOfDifference) const
{
  return this->DifferenceMap.find(TypeOfDifference(nameOfDifference))->second;
}

float PairDifferences::GetDifferenceByType(const PatchDifferenceTypes differenceType) const
{
  return this->DifferenceMap.find(differenceType)->second;
}

std::vector<std::string> PairDifferences::GetDifferenceNames() const
{
  std::vector<std::string> names;
  for(DifferenceMapType::const_iterator differenceIterator = this->DifferenceMap.begin(); differenceIterator != this->DifferenceMap.end(); ++differenceIterator)
    {
    names.push_back(NameOfDifference(differenceIterator->first));
    }
  return names;
}

void PairDifferences::Invalidate()
{
  this->DifferenceMap.clear();
}

PairDifferences::PatchDifferenceTypes PairDifferences::TypeOfDifference(const std::string& nameOfDifference)
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

std::string PairDifferences::NameOfDifference(const PatchDifferenceTypes typeOfDifference)
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
