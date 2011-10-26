#include "PatchPair.h"

void PatchPair::DefaultConstructor()
{
  this->AverageSSD = 0.0f;
  this->BoundaryIsophoteDifference = 0.0f;
  this->BoundaryPixelDifference = 0.0f;
  this->TotalScore = 0.0f;
  
  this->ValidSSD = false;
  this->ValidBoundaryPixelDifference = false;
  this->ValidBoundaryIsophoteDifference = false;
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

bool PatchPair::IsValidBoundaryIsophoteDifference()
{
  return ValidBoundaryIsophoteDifference;
}

  

bool SortByAverageSSD(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetAverageSSD() < pair2.GetAverageSSD());
}

bool SortByBoundaryPixelDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetBoundaryPixelDifference() < pair2.GetBoundaryPixelDifference());
}

bool SortByBoundaryIsophoteDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.GetBoundaryIsophoteDifference() < pair2.GetBoundaryIsophoteDifference());
}

void PatchPair::SetAverageSSD(const float value)
{
  this->ValidSSD = true;
  this->AverageSSD = value;
  ComputeTotal();
}

void PatchPair::SetBoundaryPixelDifference(const float value)
{
  this->ValidBoundaryPixelDifference = true;
  this->BoundaryPixelDifference = value;
  ComputeTotal();
}

void PatchPair::SetBoundaryIsophoteDifference(const float value)
{
  this->ValidBoundaryIsophoteDifference = true;
  this->BoundaryIsophoteDifference = value;
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

float PatchPair::GetBoundaryPixelDifference() const
{
  return this->BoundaryPixelDifference;
}

float PatchPair::GetBoundaryIsophoteDifference() const
{
  return this->BoundaryIsophoteDifference;
}

void PatchPair::ComputeTotal()
{
  this->TotalScore = this->BoundaryIsophoteDifference + this->BoundaryPixelDifference + this->AverageSSD;
}
