#include "PatchPair.h"

PatchPair::PatchPair()
{
  this->AverageSSD = 0.0f;
  this->BoundaryIsophoteDifference = 0.0f;
  this->BoundaryPixelDifference = 0.0f;
  this->TotalScore = 0.0f;
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
  this->AverageSSD = value;
  ComputeTotal();
}

void PatchPair::SetBoundaryPixelDifference(const float value)
{
  this->BoundaryPixelDifference = value;
  ComputeTotal();
}

void PatchPair::SetBoundaryIsophoteDifference(const float value)
{
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
