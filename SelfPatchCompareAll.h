#ifndef SelfPatchCompareAll_H
#define SelfPatchCompareAll_H

#include "SelfPatchCompare.h"

class SelfPatchCompareAll : public SelfPatchCompare
{
public:
  
  SelfPatchCompareAll(const unsigned int components) : SelfPatchCompare(components){}
  
  float PixelDifference(const VectorType &a, const VectorType &b)
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
};

#endif
