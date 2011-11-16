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

#ifndef PATCHPAIR_H
#define PATCHPAIR_H

#include "Patch.h"

struct PatchPair
{
  void DefaultConstructor();
  PatchPair();
  PatchPair(const Patch& sourcePatch, const Patch& targetPatch);
  
  Patch SourcePatch;
  Patch TargetPatch;
  
  // All of these different scores must use accessors/mutators because we might want to recompute other scores if one of the scores changes.
  // Also, we can avoid recomputation if a container of PatchPairs is asked to compute everything - we would only compute each difference
  // if it is invalid (i.e. not already computed).
  
  // Difference accessors
  float GetAverageSquaredDifference() const;
  float GetAverageAbsoluteDifference() const;
  float GetBoundaryGradientDifference() const;
  float GetBoundaryPixelDifference() const;
  float GetBoundaryIsophoteAngleDifference() const;
  float GetBoundaryIsophoteStrengthDifference() const;
  float GetColorDifference() const;
  float GetDepthDifference() const;
  float GetTotalScore() const;

  float GetDepthAndColorDifference() const;
  
  // Difference mutators
  void SetAverageSquaredDifference(const float value);
  void SetAverageAbsoluteDifference(const float value);
  void SetBoundaryGradientDifference(const float value);
  void SetBoundaryPixelDifference(const float value);
  void SetBoundaryIsophoteAngleDifference(const float value);
  void SetBoundaryIsophoteStrengthDifference(const float value);
  void SetColorDifference(const float value);
  void SetDepthDifference(const float value);
  
  // Valid accessors
  bool IsValidAverageSquaredDifference() const;
  bool IsValidAverageAbsoluteDifference() const;
  bool IsValidBoundaryGradientDifference() const;
  bool IsValidBoundaryPixelDifference() const;
  bool IsValidBoundaryIsophoteAngleDifference() const;
  bool IsValidBoundaryIsophoteStrengthDifference() const;
  bool IsValidColorDifference() const;
  bool IsValidDepthDifference() const;
  bool IsValidDepthAndColorDifference() const;
  
  // Valid mutators
  void SetValidAverageSquaredDifference(const bool);
  void SetValidAverageAbsoluteDifference(const bool);
  void SetValidBoundaryGradientDifference(const bool);
  void SetValidBoundaryPixelDifference(const bool);
  void SetValidBoundaryIsophoteAngleDifference(const bool);
  void SetValidBoundaryIsophoteStrengthDifference(const bool);
  void SetValidColorDifference(const bool);
  void SetValidDepthDifference(const bool);
  
  //float GetSortValue();

  itk::Offset<2> GetTargetToSourceOffset() const;
  itk::Offset<2> GetSourceToTargetOffset() const;
  
  static float DepthColorLambda;
  
private:
  float AverageSquaredDifference;
  float AverageAbsoluteDifference;
  float BoundaryGradientDifference;
  float BoundaryPixelDifference;
  float BoundaryIsophoteAngleDifference;
  float BoundaryIsophoteStrengthDifference;
  float DepthDifference;
  float ColorDifference;
  float DepthAndColorDifference;
  
  float TotalScore;
  
  void ComputeTotal();
  void ComputeDepthAndColorDifference();
  
  // These are initialized to false and set to true when the corresponding SetXYZ() function is called.
  bool ValidAverageSquaredDifference;
  bool ValidAverageAbsoluteDifference;
  bool ValidBoundaryGradientDifference;
  bool ValidBoundaryPixelDifference;
  bool ValidBoundaryIsophoteAngleDifference;
  bool ValidBoundaryIsophoteStrengthDifference;
  bool ValidColorDifference;
  bool ValidDepthDifference;
  
  //static float DepthColorLambda;
};

// struct SortFunctor
// {
//   bool operator()(const Test &T1, const Test &T2)
//   {
//     return(T1.a < T2.a);
//   }
//   
//   boost::function<bool (const PatchPair& , const PatchPair& )> PatchSortFunction;
// };

// Sorting functions
bool SortByAverageSquaredDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByAverageAbsoluteDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryGradientDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryIsophoteAngleDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryIsophoteStrengthDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryPixelDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByDepthDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByTotalScore(const PatchPair& pair1, const PatchPair& pair2);
bool SortByDepthAndColor(const PatchPair& pair1, const PatchPair& pair2);

#endif
