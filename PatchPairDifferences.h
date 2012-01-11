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

#ifndef PatchPairDifferences_H
#define PatchPairDifferences_H

#include <map>
#include <string>
#include <vector>

/**
\class PatchPairDifferences
\brief Store different types of differences.
*/
class PatchPairDifferences
{
public:
  /** The list of possible difference types.*/
  enum PatchPairDifferenceTypes {SumPixelDifference, AveragePixelDifference, ColorDifference, DepthDifference, Invalid};

  /** Get a string representing a different type.*/
  static std::string NameOfDifference(const PatchPairDifferenceTypes);

  /** Get the difference type from a string.*/
  static PatchPairDifferenceTypes TypeOfDifference(const std::string&);

  /** Get the number of differences current held by this object.*/
  unsigned int GetNumberOfDifferences() const;

  /** Get the strings of all differences currently held by this object.*/
  std::vector<std::string> GetDifferenceNames() const;

  /** Get the value of a specific type of difference.*/
  float GetDifferenceByName(const std::string& differenceName) const;

  /** Get the value of a specific type of difference.*/
  float GetDifferenceByType(const PatchPairDifferenceTypes differenceType) const;

  /** Set the value of a specific type of difference.*/
  void SetDifferenceByName(const std::string& differenceName, const float value);

  /** Set the value of a specific type of difference.*/
  void SetDifferenceByType(const PatchPairDifferenceTypes differenceType, const float value);

  /** Clear the object.*/
  void Invalidate();

private:
  typedef std::map <PatchPairDifferenceTypes, float> DifferenceMapType;
  DifferenceMapType DifferenceMap;

  typedef std::map <PatchPairDifferenceTypes, std::string> DifferenceNameMapType;
  static DifferenceNameMapType DifferenceNameMap;
  static DifferenceNameMapType CreateNameMap();

};

#endif
