#ifndef PATCHPAIR_H
#define PATCHPAIR_H

#include "Patch.h"

struct PatchPair
{
  void DefaultConstructor();
  PatchPair();
  PatchPair(const Patch&, const Patch&);
  
  Patch SourcePatch;
  Patch TargetPatch;
  
  float GetAverageSSD() const;
  float GetBoundaryPixelDifference() const;
  float GetBoundaryIsophoteDifference() const;
  float GetTotalScore() const;
  
  void SetAverageSSD(const float value);
  void SetBoundaryPixelDifference(const float value);
  void SetBoundaryIsophoteDifference(const float value);
  
  bool IsValidSSD();
  bool IsValidBoundaryPixelDifference();
  bool IsValidBoundaryIsophoteDifference();
  
private:
  float AverageSSD;
  float BoundaryPixelDifference;
  float BoundaryIsophoteDifference;
  
  float TotalScore;
  
  void ComputeTotal();
  
  // These are initialized to false and set to true when the corresponding SetXYZ() function is called.
  bool ValidSSD;
  bool ValidBoundaryPixelDifference;
  bool ValidBoundaryIsophoteDifference;
};

bool SortByAverageSSD(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryIsophoteDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryPixelDifference(const PatchPair& pair1, const PatchPair& pair2);

#endif
