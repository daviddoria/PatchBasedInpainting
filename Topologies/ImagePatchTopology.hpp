#ifndef ImagePatchTopology_HPP
#define ImagePatchTopology_HPP

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

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
    return Compare(&a, &b);
  }

};

#endif
