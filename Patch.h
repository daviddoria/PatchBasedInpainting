#ifndef PATCH_H
#define PATCH_H

#include "Types.h"

struct Patch
{
public:
  Patch(){} // This is needed to allow a std::vector<Patch> to be constructed
  Patch(const itk::ImageRegion<2>& region);
  
  // This is needed if we are computing histograms upon construction.
  //Patch(const FloatVectorImageType::Pointer image, const itk::ImageRegion<2>& region);
  
  // The region in the image defining the patch.
  itk::ImageRegion<2> Region;

  //float SortValue; // This simply allows patches to be sorted by any criterion currently being evaluated.
  //unsigned int Id; // This is used for parallel sorting of patches.
};

//bool SortBySortValue(const Patch& patch1, const Patch& patch2);

#endif
