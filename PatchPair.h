#ifndef PATCHPAIR_H
#define PATCHPAIR_H

#include "Patch.h"

struct PatchPair
{
  PatchPair();
  
  Patch SourcePatch;
  Patch TargetPatch;
  
  float GetAverageSSD() const;
  float GetBoundaryPixelDifference() const;
  float GetBoundaryIsophoteDifference() const;
  float GetTotalScore() const;
  
  void SetAverageSSD(const float value);
  void SetBoundaryPixelDifference(const float value);
  void SetBoundaryIsophoteDifference(const float value);
  
private:
  float AverageSSD;
  float BoundaryPixelDifference;
  float BoundaryIsophoteDifference;
  
  float TotalScore;
  
  void ComputeTotal();
};

bool SortByAverageSSD(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryIsophoteDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryPixelDifference(const PatchPair& pair1, const PatchPair& pair2);

#endif
