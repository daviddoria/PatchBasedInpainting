#ifndef SelfPatchCompareColor_H
#define SelfPatchCompareColor_H

#include "SelfPatchCompare.h"

#include "CandidatePairs.h"

class SelfPatchCompareColor : public SelfPatchCompare
{
public:
  SelfPatchCompareColor(const unsigned int components, CandidatePairs& candidatePairs) : SelfPatchCompare(components, candidatePairs) 
  {
    if(components < 3)
      {
      std::cerr << "Cannot compute color differences without at least 3 components!" << std::endl;
      exit(-1);
      }
  }
  
  float PixelDifference(const VectorType &a, const VectorType &b)
  {
    float difference = 0;
    
    float diff = 0;
    for(unsigned int i = 0; i < 3; ++i)
      {
      diff = a[i] - b[i];
      //difference += diff * diff; // Squared difference
      difference += abs(diff); // Absolute difference
      }
    return difference;
  }
};

#endif
