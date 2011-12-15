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

#include "PatchPair.h"

void PatchPair::DefaultConstructor()
{

}

PatchPair::PatchPair()
{
  DefaultConstructor();
}

PatchPair::PatchPair(const Patch& sourcePatch, const Patch& targetPatch)
{
  DefaultConstructor();

  this->SourcePatch = sourcePatch;
  this->TargetPatch = targetPatch;
}

std::string PatchPair::NameOfDifference(PatchDifferenceTypes enumValue)
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

itk::Offset<2> PatchPair::GetTargetToSourceOffset() const
{
  return this->SourcePatch.Region.GetIndex() - this->TargetPatch.Region.GetIndex();
}

itk::Offset<2> PatchPair::GetSourceToTargetOffset() const
{
  return this->TargetPatch.Region.GetIndex() - this->SourcePatch.Region.GetIndex();
}

float PatchPair::GetDepthAndColorDifference() const
{
  DifferenceMapType::const_iterator colorIter = this->DifferenceMap.find(ColorDifference);

  if(colorIter == this->DifferenceMap.end())
    {
    std::cerr << "Could not compute GetDepthAndColorDifference, ColorDifference not found." << std::endl;
    exit(-1);
    }

  DifferenceMapType::const_iterator depthIter = this->DifferenceMap.find(DepthDifference);

  if(depthIter == this->DifferenceMap.end())
    {
    std::cerr << "Could not compute GetDepthAndColorDifference, DepthDifference not found." << std::endl;
    exit(-1);
    }

  return ComputeDepthAndColorDifference(this->DifferenceMap.find(DepthDifference)->second, this->DifferenceMap.find(ColorDifference)->second);
}

//////////////////////////////////////////
/////// Computations (functions of more than one difference) //////////////
//////////////////////////////////////////

float PatchPair::ComputeDepthAndColorDifferenceFunctor::operator()(const float depthDifference, const float colorDifference) const
{
  //this->DepthAndColorDifference = this->ColorDifference * DepthColorLambda + (1.0 - DepthColorLambda) * this->DepthDifference;
  return depthDifference + this->DepthColorLambda * colorDifference;
}
