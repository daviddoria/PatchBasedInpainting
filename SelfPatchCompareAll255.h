#ifndef SelfPatchCompareAll255_H
#define SelfPatchCompareAll255_H

#include "SelfPatchCompare.h"

class SelfPatchCompareAll255 : public SelfPatchCompare
{
public:
  SelfPatchCompareAll255(const unsigned int components) : SelfPatchCompare(components){}
  
  float PixelDifference(const VectorType &a, const VectorType &b)
  {
    float difference = 0;
    
    float diff = 0;
    for(unsigned int i = 0; i < 3; ++i)
      {
      diff = a[i] - b[i];
      difference += diff * diff;
      }
      
    // Assuming a range of (0-10), to rescale to (0,255) all we have to do is multiply by 25.
    for(unsigned int i = 3; i < this->NumberOfComponentsPerPixel; ++i)
      {
      diff = 25* (a[i] - b[i]);
      difference += diff * diff;
      }
    return difference;
  }
};

#endif
