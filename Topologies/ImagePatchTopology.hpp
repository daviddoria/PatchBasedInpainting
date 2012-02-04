#ifndef ImagePatchTopology_HPP
#define ImagePatchTopology_HPP

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"
#include "DifferenceFunctions/ImagePatchDifference.hpp"

template <typename TImage>
class ImagePatchTopology
{
public:
  typedef ImagePatchPixelDescriptor<TImage> point_type;
  typedef float point_difference_type;

  /**
    * Returns the distance between two points.
    */
  double distance(const point_type& a, const point_type& b) const 
  {
    return DifferenceFunction(a, b);
  }

private:
  ImagePatchDifference<point_type> DifferenceFunction;
};

#endif
