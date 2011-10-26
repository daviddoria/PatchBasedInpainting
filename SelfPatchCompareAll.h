#ifndef SelfPatchCompareAll_H
#define SelfPatchCompareAll_H

#include "SelfPatchCompare.h"

class SelfPatchCompareAll : public SelfPatchCompare
{
public:
  
  SelfPatchCompareAll(const unsigned int components, CandidatePairs& candidatePairs) : SelfPatchCompare(components, candidatePairs)
  {
    // Nothing special to do in this constructor - everything is done in SelfPatchCompare constructor.
  }

  float PixelDifference(const VectorType &a, const VectorType &b)
  {
    float difference = 0;

    float diff = 0;
    for(unsigned int i = 0; i < this->NumberOfComponentsPerPixel; ++i)
      {
      diff = fabs(a[i] - b[i]);
      difference += diff;
      }
    return difference;
  }
  
  float PixelDifferenceSquared(const VectorType &a, const VectorType &b)
  {
    float difference = 0;
    
    float diff = 0;
    for(unsigned int i = 0; i < this->NumberOfComponentsPerPixel; ++i)
      {
      diff = a[i] - b[i];
      difference += diff * diff;
      }
    return difference;
  }
  
  static float StaticPixelDifferenceSquared(const VectorType &a, const VectorType &b)
  {
    float difference = 0;
    
    float diff = 0;
    for(unsigned int i = 0; i < a.GetNumberOfElements(); ++i)
      {
      diff = a[i] - b[i];
      difference += diff * diff;
      }
    return difference;
  }
};

#endif
