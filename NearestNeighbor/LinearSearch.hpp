#ifndef LinearSearch_HPP
#define LinearSearch_HPP

#include <limits>

// Boost
#include <boost/property_map/property_map.hpp>

struct LinearSearch
{
  template <typename TVertex, typename TGraph, typename TTopology, typename TPositionMap>
  TVertex operator()(const TVertex target_patch_center, TGraph& g, TTopology space, TPositionMap positionMap )
  {
    float smallestDistance = std::numeric_limits<float>::max();

    typedef typename boost::property_traits<TPositionMap>::value_type PositionType;
    
    for(TVertex v = 0; v < num_vertices(g); ++v) 
    {
      TVertex currentVertex = get(positionMap, g, v);
    }
  }
};

#endif
