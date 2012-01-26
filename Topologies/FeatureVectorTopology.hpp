#ifndef FeatureVectorTopology_h
#define FeatureVectorTopology_h

#include <vector>

class FeatureVectorTopology
{
public:
  typedef std::vector<float> point_type;
  typedef float point_difference_type;

  /**
    * Returns the distance between two points.
    */
  double distance(point_type a, point_type b) const
  {
    assert(a.size() == b.size());
    double dist = 0.0f;
    for(unsigned int i = 0; i < a.size(); ++i)
      {
      dist += fabs(a[i] - b[i]);
      }
    return dist;
  }
};

#endif
