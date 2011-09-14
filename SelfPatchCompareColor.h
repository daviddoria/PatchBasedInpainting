#ifndef SelfPatchCompareColor_H
#define SelfPatchCompareColor_H

#include "SelfPatchCompare.h"

class SelfPatchCompareColor : public SelfPatchCompare
{
public:
  SelfPatchCompareColor(const unsigned int components) : SelfPatchCompare(components) {}
  
  float PixelDifference(const VectorType &a, const VectorType &b)
  {
    float difference = 0;
    
    float diff = 0;
    for(unsigned int i = 0; i < 3; ++i)
      {
      diff = a[i] - b[i];
      difference += diff * diff;
      }
    return difference;
  }
};

#endif
