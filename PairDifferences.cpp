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

void PairDifferences::SetDifferenceByName(const std::string& differenceName, const float value)
{
  this->DifferenceMap.find(TypeOfDifference(differenceName))->second = value;
}

void PairDifferences::SetDifferenceByType(const PatchDifferenceTypes differenceType, const float value)
{
  this->DifferenceMap.find(differenceType)->second = value;
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
  for(DifferenceMapType::const_iterator it = this->DifferenceMap.begin(); it != this->DifferenceMap.end(); ++it)
    {
    names.push_back(NameOfDifference(it->first));
    }
  return names;
}

void PairDifferences::Invalidate()
{
  this->DifferenceMap.clear();
}

PairDifferences::PatchDifferenceTypes PairDifferences::TypeOfDifference(const std::string& nameOfDifference)
{
  if(nameOfDifference == "Av.Abs.")
    {
    return AverageAbsoluteDifference;
    }
  else if(nameOfDifference == "Color")
    {
    return ColorDifference;
    }
  else if(nameOfDifference == "Depth")
    {
    return DepthDifference;
    }
  else if(nameOfDifference == "Membership")
    {
    return MembershipDifference;
    }
  else if(nameOfDifference == "Combined")
    {
    return CombinedDifference;
    }
  else if(nameOfDifference == "Hist.Int.")
    {
    return HistogramIntersection;
    }
  else
    {
    return Invalid;
    }
}

std::string PairDifferences::NameOfDifference(const PatchDifferenceTypes enumValue)
{
  std::string namedDifference;
  switch(enumValue)
  {
    case AverageAbsoluteDifference:
      //namedDifference = "AverageAbsoluteDifference";
      namedDifference = "Av.Abs.";
      break;
    case ColorDifference:
      //namedDifference = "ColorDifference";
      namedDifference = "Color";
      break;
    case DepthDifference:
      //namedDifference = "DepthDifference";
      namedDifference = "Depth";
      break;
    case MembershipDifference:
      //namedDifference = "MembershipDifference";
      namedDifference = "Membership";
      break;
    case CombinedDifference:
      //namedDifference = "CombinedDifference";
      namedDifference = "Combined";
      break;
    case HistogramIntersection:
      namedDifference = "Hist.Int.";
      break;
    default:
      namedDifference = "INVALID";
      break;
  }

  return namedDifference;
}
