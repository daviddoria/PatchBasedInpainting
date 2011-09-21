#ifndef PATCH_H
#define PATCH_H

#include "Types.h"

class Patch
{
public:
  Patch(){} // This is needed to allow a std::vector<Patch> to be constructed
  Patch(const itk::ImageRegion<2>& region);
  
  // This is needed if we are computing histograms upon construction.
  //Patch(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region);
  
//protected:
  itk::ImageRegion<2> Region;
  std::vector<ImageToHistogramFilterType::HistogramType::Pointer> Histograms;
};

#endif
