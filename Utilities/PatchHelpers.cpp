#include "PatchHelpers.h"

namespace PatchHelpers
{
  bool CheckSurroundingRegionsOfAllHolePixels(const Mask* const mask, const unsigned int patchRadius)
  {
    std::vector<itk::Index<2> > maskPixels = mask->GetHolePixels();

    for(size_t i = 0; i < maskPixels.size(); ++i)
    {
      itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(maskPixels[i], patchRadius);
      if(!mask->GetLargestPossibleRegion().IsInside(region))
      {
        return false;
      }
    }

    return true;
  }

} // end PatchHelpers namespace
