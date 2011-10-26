#include "PatchPair.h"

void PatchPair::DefaultConstructor()
{
  this->AverageSSD = 0.0f;
  this->TotalAbsoluteDifference = 0.0f;
  this->BoundaryIsophoteAngleDifference = 0.0f;
  this->BoundaryIsophoteStrengthDifference = 0.0f;
  this->BoundaryPixelDifference = 0.0f;
  this->TotalScore = 0.0f;
  
  this->ValidSSD = false;
  this->ValidTotalAbsoluteDifference = false;
  this->ValidBoundaryPixelDifference = false;
  this->ValidBoundaryIsophoteAngleDifference = false;
  this->ValidBoundaryIsophoteStrengthDifference = false;
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
  
bool PatchPair::IsValidSSD()
{
  return ValidSSD;
}

bool PatchPair::IsValidBoundaryPixelDifference()
{
  return ValidBoundaryPixelDifference;
}

bool PatchPair::IsValidBoundaryIsophoteAngleDifference()
{
  return ValidBoundaryIsophoteAngleDifference;
}

bool PatchPair::IsValidBoundaryIsophoteStrengthDifference()
{
  return ValidBoundaryIsophoteStrengthDifference;
}

bool SortByAverageSSD(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetAverageSSD() < pair2.GetAverageSSD());
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

void PatchPair::SetAverageSSD(const float value)
{
  this->ValidSSD = true;
  this->AverageSSD = value;
  ComputeTotal();
}

void PatchPair::SetTotalAbsoluteDifference(const float value)
{
  this->ValidTotalAbsoluteDifference = true;
  this->TotalAbsoluteDifference = value;
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

float PatchPair::GetTotalScore() const
{
  return this->TotalScore;
}

float PatchPair::GetAverageSSD() const
{
  return this->AverageSSD;
}

float PatchPair::GetTotalAbsoluteDifference() const
{
  return this->TotalAbsoluteDifference;
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

void PatchPair::ComputeTotal()
{
  this->TotalScore = this->BoundaryIsophoteStrengthDifference + this->BoundaryIsophoteAngleDifference + this->BoundaryPixelDifference + this->AverageSSD;
}
