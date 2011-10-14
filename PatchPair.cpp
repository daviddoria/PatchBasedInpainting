#include "PatchPair.h"

bool SortByAverageSSD(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.AverageSSD < pair2.AverageSSD);
}

bool SortByHistogramDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.HistogramDifference < pair2.HistogramDifference);
}

bool SortByContinuationDifference(const PatchPair& pair1, const PatchPair& pair2)
{
  return (pair1.ContinuationDifference < pair2.ContinuationDifference);
}
