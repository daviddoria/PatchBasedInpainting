#ifndef SelfPatchCompareDepth_H
#define SelfPatchCompareDepth_H

#include "SelfPatchCompare.h"

class SelfPatchCompareDepth : public SelfPatchCompare
{
public:
  SelfPatchCompareDepth(const unsigned int numberOfComponents) : SelfPatchCompare(numberOfComponents)
  {
    if(numberOfComponents < 4)
      {
      std::cerr << "Must have at least 4 components to use SelfPatchCompareDepth!" << std::endl;
      exit(-1);
      }
  }
  
//public:
  float PixelDifference(const VectorType &a, const VectorType &b)
  {
    float difference = 0;
    
    float diff = 0;
    // Compute the squared norm of the difference of the color channels
    for(unsigned int i = 0; i < 3; ++i)
      {
      diff = a[i] - b[i];
      difference += diff * diff;
      }
    
    //std::cout << "difference was: " << difference << std::endl;
    
    float depthDifference = fabs(a[3] - b[3]);
    
    //std::cout << "depthDifference : " << depthDifference << std::endl;
    
    difference += this->MaxColorDifference * (1.-exp(-depthDifference));
    //difference += this->MaxColorDifference * depthDifference;
    
    //std::cout << "difference is now: " << difference << std::endl;
    
    return difference;
  }
};

#endif
