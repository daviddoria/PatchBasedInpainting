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

#ifndef PairDifferences_H
#define PairDifferences_H

#include <map>
#include <string>
#include <vector>

class PairDifferences
{
public:
  enum PatchDifferenceTypes {SumPixelDifference, AveragePixelDifference, ColorDifference, DepthDifference, Invalid};

  static std::string NameOfDifference(const PatchDifferenceTypes);
  static PatchDifferenceTypes TypeOfDifference(const std::string&);

  unsigned int GetNumberOfDifferences() const;

  std::vector<std::string> GetDifferenceNames() const;

  float GetDifferenceByName(const std::string& differenceName) const;
  float GetDifferenceByType(const PatchDifferenceTypes differenceType) const;

  void SetDifferenceByName(const std::string& differenceName, const float value);
  void SetDifferenceByType(const PatchDifferenceTypes differenceType, const float value);

  void Invalidate();

private:
  typedef std::map <PatchDifferenceTypes, float> DifferenceMapType;
  DifferenceMapType DifferenceMap;

  typedef std::map <PatchDifferenceTypes, std::string> DifferenceNameMapType;
  static DifferenceNameMapType DifferenceNameMap;
  static DifferenceNameMapType CreateNameMap();

};

#endif
