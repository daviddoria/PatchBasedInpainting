#ifndef PATCHPAIR_H
#define PATCHPAIR_H

#include "Patch.h"

struct PatchPair
{
  Patch SourcePatch;
  Patch TargetPatch;
  float AverageSSD;
  float HistogramDifference;
  float ContinuationDifference;
};

bool SortByAverageSSD(const PatchPair& pair1, const PatchPair& pair2);
bool SortByHistogramDifference(const PatchPair& pair1, const PatchPair& pair2);
bool SortByContinuationDifference(const PatchPair& pair1, const PatchPair& pair2);

#endif
