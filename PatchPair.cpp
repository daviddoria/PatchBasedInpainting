#include "PatchPair.h"

float PatchPair::DepthColorLambda = 0.5f;

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
  this->DepthAndColorDifference = 0.0f;
  this->TotalScore = 0.0f;
  
  //this->DepthColorLambda = 0.5f;
  
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
/////// Sorting functions //////////////
//////////////////////////////////////////
bool SortByDepthAndColor(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetDepthAndColorDifference() < pair2.GetDepthAndColorDifference());
}

bool SortByAverageSquaredDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetAverageSquaredDifference() < pair2.GetAverageSquaredDifference());
}

bool SortByAverageAbsoluteDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetAverageAbsoluteDifference() < pair2.GetAverageAbsoluteDifference());
}

bool SortByBoundaryGradientDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetBoundaryGradientDifference() < pair2.GetBoundaryGradientDifference());
}

bool SortByBoundaryPixelDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetBoundaryPixelDifference() < pair2.GetBoundaryPixelDifference());
}

bool SortByBoundaryIsophoteAngleDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetBoundaryIsophoteAngleDifference() < pair2.GetBoundaryIsophoteAngleDifference());
}

bool SortByBoundaryIsophoteStrengthDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetBoundaryIsophoteStrengthDifference() < pair2.GetBoundaryIsophoteStrengthDifference());
}

bool SortByTotalScore(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetTotalScore() < pair2.GetTotalScore());
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
  ComputeDepthAndColorDifference();
}

void PatchPair::SetDepthDifference(const float value)
{
  this->ValidDepthDifference = true;
  this->DepthDifference = value;
  ComputeTotal();
  ComputeDepthAndColorDifference();
}

//////////////////////////////////////////
/////// Difference accessors //////////////
//////////////////////////////////////////

float PatchPair::GetTotalScore() const
{
  return this->TotalScore;
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
  return this->DepthAndColorDifference;
}

//////////////////////////////////////////
/////// Computations (functions of more than one difference) //////////////
//////////////////////////////////////////
void PatchPair::ComputeTotal()
{
  //this->TotalScore = this->BoundaryIsophoteStrengthDifference + this->BoundaryIsophoteAngleDifference + this->BoundaryPixelDifference + this->AverageSSD + this->BoundaryGradientDifference;
}

void PatchPair::ComputeDepthAndColorDifference()
{
  if(this->ValidColorDifference && this->ValidDepthDifference)
    {
    this->DepthAndColorDifference = this->ColorDifference * DepthColorLambda + (1.0 - DepthColorLambda) * this->DepthDifference;
    }
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
