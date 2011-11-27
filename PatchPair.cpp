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
  this->AverageAbsoluteDifference = 0.0f;
  this->AverageSquaredDifference = 0.0f;
  this->BoundaryGradientDifference = 0.0f;
  this->BoundaryIsophoteAngleDifference = 0.0f;
  this->BoundaryIsophoteStrengthDifference = 0.0f;
  this->BoundaryPixelDifference = 0.0f;
  this->DepthDifference = 0.0f;
  this->ColorDifference = 0.0f;
  this->TotalScore = 0.0f;
  
  this->ValidAverageSquaredDifference = false;
  this->ValidAverageAbsoluteDifference = false;
  this->ValidBoundaryGradientDifference = false;
  this->ValidBoundaryPixelDifference = false;
  this->ValidBoundaryIsophoteAngleDifference = false;
  this->ValidBoundaryIsophoteStrengthDifference = false;
  this->ValidColorDifference = false;
  this->ValidDepthDifference = false;

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

itk::Offset<2> PatchPair::GetTargetToSourceOffset() const
{
  return this->SourcePatch.Region.GetIndex() - this->TargetPatch.Region.GetIndex();
}

itk::Offset<2> PatchPair::GetSourceToTargetOffset() const
{
  return this->TargetPatch.Region.GetIndex() - this->SourcePatch.Region.GetIndex();
}

//////////////////////////////////////////
/////// Valid accessors //////////////
//////////////////////////////////////////
bool PatchPair::IsValidDepthDifference() const
{
  return ValidDepthDifference;
}

bool PatchPair::IsValidColorDifference() const
{
  return ValidColorDifference;
}

bool PatchPair::IsValidDepthAndColorDifference() const
{
  return ValidColorDifference && ValidDepthDifference;
}

bool PatchPair::IsValidAverageSquaredDifference() const
{
  return ValidAverageSquaredDifference;
}

bool PatchPair::IsValidAverageAbsoluteDifference() const
{
  return ValidAverageAbsoluteDifference;
}

bool PatchPair::IsValidBoundaryGradientDifference() const
{
  return ValidBoundaryPixelDifference;
}

bool PatchPair::IsValidBoundaryPixelDifference() const
{
  return ValidBoundaryPixelDifference;
}

bool PatchPair::IsValidBoundaryIsophoteAngleDifference() const
{
  return ValidBoundaryIsophoteAngleDifference;
}

bool PatchPair::IsValidBoundaryIsophoteStrengthDifference() const
{
  return ValidBoundaryIsophoteStrengthDifference;
}

//////////////////////////////////////////
/////// Difference mutators //////////////
//////////////////////////////////////////
void PatchPair::SetAverageSquaredDifference(const float value)
{
  this->ValidAverageSquaredDifference = true;
  this->AverageSquaredDifference = value;
  ComputeTotal();
}

void PatchPair::SetAverageAbsoluteDifference(const float value)
{
  this->ValidAverageAbsoluteDifference = true;
  this->AverageAbsoluteDifference = value;
  ComputeTotal();
}

void PatchPair::SetBoundaryGradientDifference(const float value)
{
  this->ValidBoundaryGradientDifference = true;
  this->BoundaryGradientDifference = value;
  ComputeTotal();
}

void PatchPair::SetBoundaryPixelDifference(const float value)
{
  this->ValidBoundaryPixelDifference = true;
  this->BoundaryPixelDifference = value;
  ComputeTotal();
}

void PatchPair::SetBoundaryIsophoteAngleDifference(const float value)
{
  this->ValidBoundaryIsophoteAngleDifference = true;
  this->BoundaryIsophoteAngleDifference = value;
  ComputeTotal();
}

void PatchPair::SetBoundaryIsophoteStrengthDifference(const float value)
{
  this->ValidBoundaryIsophoteStrengthDifference = true;
  this->BoundaryIsophoteStrengthDifference = value;
  ComputeTotal();
}

void PatchPair::SetColorDifference(const float value)
{
  this->ValidColorDifference = true;
  this->ColorDifference = value;
  ComputeTotal();
}

void PatchPair::SetDepthDifference(const float value)
{
  this->ValidDepthDifference = true;
  this->DepthDifference = value;
  ComputeTotal();
}

//////////////////////////////////////////
/////// Difference accessors //////////////
//////////////////////////////////////////

float PatchPair::GetTotalScore() const
{
  return this->TotalScore;
}

float PatchPair::GetDepthDifference() const
{
  return this->DepthDifference;
}

float PatchPair::GetColorDifference() const
{
  return this->ColorDifference;
}

float PatchPair::GetAverageSquaredDifference() const
{
  return this->AverageSquaredDifference;
}

float PatchPair::GetAverageAbsoluteDifference() const
{
  return this->AverageAbsoluteDifference;
}

float PatchPair::GetBoundaryGradientDifference() const
{
  return this->BoundaryGradientDifference;
}

float PatchPair::GetBoundaryPixelDifference() const
{
  return this->BoundaryPixelDifference;
}

float PatchPair::GetBoundaryIsophoteAngleDifference() const
{
  return this->BoundaryIsophoteAngleDifference;
}

float PatchPair::GetBoundaryIsophoteStrengthDifference() const
{
  return this->BoundaryIsophoteStrengthDifference;
}

float PatchPair::GetDepthAndColorDifference() const
{
  if(!this->ValidColorDifference && this->ValidDepthDifference)
    {
    std::cerr << "PatchPair::GetDepthAndColorDifference(): Both ColorDifference and DepthDifference must have been calculated!" << std::endl;
    exit(-1);
    }
  return ComputeDepthAndColorDifference(this->DepthDifference, this->ColorDifference);
}

//////////////////////////////////////////
/////// Computations (functions of more than one difference) //////////////
//////////////////////////////////////////
void PatchPair::ComputeTotal()
{
  //this->TotalScore = this->BoundaryIsophoteStrengthDifference + this->BoundaryIsophoteAngleDifference + this->BoundaryPixelDifference + this->AverageSSD + this->BoundaryGradientDifference;
}

float PatchPair::ComputeDepthAndColorDifferenceFunctor::operator()(const float depthDifference, const float colorDifference) const
{
  //this->DepthAndColorDifference = this->ColorDifference * DepthColorLambda + (1.0 - DepthColorLambda) * this->DepthDifference;
  return depthDifference + this->DepthColorLambda * colorDifference;
}

//////////////////////////////////////////
/////// Valid mutators //////////////
//////////////////////////////////////////
void PatchPair::SetValidAverageSquaredDifference(const bool value)
{
  this->ValidAverageSquaredDifference = value;
}

void PatchPair::SetValidAverageAbsoluteDifference(const bool value)
{
  this->ValidAverageAbsoluteDifference = value;
}

void PatchPair::SetValidBoundaryGradientDifference(const bool value)
{
  this->ValidBoundaryGradientDifference = value;
}

void PatchPair::SetValidBoundaryPixelDifference(const bool value)
{
  this->ValidBoundaryPixelDifference = value;
}

void PatchPair::SetValidBoundaryIsophoteAngleDifference(const bool value)
{
  this->ValidBoundaryIsophoteAngleDifference = value;
}

void PatchPair::SetValidBoundaryIsophoteStrengthDifference(const bool value)
{
  this->ValidBoundaryIsophoteStrengthDifference = value;
}

void PatchPair::SetValidDepthDifference(const bool value)
{
  this->ValidBoundaryIsophoteStrengthDifference = value;
}

void PatchPair::SetValidColorDifference(const bool value)
{
  this->ValidBoundaryIsophoteStrengthDifference = value;
}
