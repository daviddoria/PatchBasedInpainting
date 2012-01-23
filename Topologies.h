#ifndef Topologies_h
#define Topologies_h

#include <boost/graph/topology.hpp>

class FeatureVectorTopology : public boost::hypercube_topology<6, boost::minstd_rand>
{
public:
  double distance(point a, point b) const
  {
    double dist = 0.0f;
    for(unsigned int i = 0; i < 6; ++i)
      {
      dist += fabs(a[i] - b[i]);
      }
    return dist;
  }
};

#endif
