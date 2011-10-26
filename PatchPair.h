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
  float GetBoundaryIsophoteAngleDifference() const;
  float GetBoundaryIsophoteStrengthDifference() const;
  float GetTotalScore() const;
  
  void SetAverageSSD(const float value);
  void SetBoundaryPixelDifference(const float value);
  void SetBoundaryIsophoteAngleDifference(const float value);
  void SetBoundaryIsophoteStrengthDifference(const float value);
  
  bool IsValidSSD();
  bool IsValidBoundaryPixelDifference();
  bool IsValidBoundaryIsophoteAngleDifference();
  bool IsValidBoundaryIsophoteStrengthDifference();

  itk::Offset<2> GetTargetToSourceOffset() const;
  itk::Offset<2> GetSourceToTargetOffset() const;
  
private:
  float AverageSSD;
  float BoundaryPixelDifference;
  float BoundaryIsophoteAngleDifference;
  float BoundaryIsophoteStrengthDifference;
  
  float TotalScore;
  
  void ComputeTotal();
  
  // These are initialized to false and set to true when the corresponding SetXYZ() function is called.
  bool ValidSSD;
  bool ValidBoundaryPixelDifference;
  bool ValidBoundaryIsophoteAngleDifference;
  bool ValidBoundaryIsophoteStrengthDifference;
};

bool SortByAverageSSD(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryIsophoteAngleDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryIsophoteStrengthDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByBoundaryPixelDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByTotalScore(const PatchPair& pair1, const PatchPair& pair2);

#endif
