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
  
//   enum PatchDifferenceTypes {AverageSquaredDifference, AverageAbsoluteDifference, BoundaryGradientDifference,
//                              BoundaryPixelDifference, BoundaryIsophoteAngleDifference, BoundaryIsophoteStrengthDifference,
//                              ColorDifference, DepthDifference, CombinedDifference};
  enum PatchDifferenceTypes {AverageAbsoluteDifference, ColorDifference, DepthDifference, CombinedDifference, MembershipDifference, HistogramIntersection};
  static std::string NameOfDifference(PatchDifferenceTypes);
  
  typedef std::map <PatchDifferenceTypes, float> DifferenceMapType;
  DifferenceMapType DifferenceMap;

  // These differences are computed as combinations of other differences
  float GetDepthAndColorDifference() const;

  // Store the relative location of the source and target patch corners
  itk::Offset<2> GetTargetToSourceOffset() const;
  itk::Offset<2> GetSourceToTargetOffset() const;
  
private:

  struct ComputeDepthAndColorDifferenceFunctor
  {
    ComputeDepthAndColorDifferenceFunctor() : DepthColorLambda(0.5f){}
    float operator()(const float depthDifference, const float colorDifference) const;
    float DepthColorLambda;
  } ComputeDepthAndColorDifference; 

};

#endif
