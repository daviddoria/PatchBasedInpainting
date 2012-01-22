#include <iostream>

#include "metric_space_search.hpp"

namespace boost {
  enum vertex_data_t { vertex_data };

  BOOST_INSTALL_PROPERTY(vertex, data);
};

int main(int argc, char *argv[])
{
  typedef boost::rectangle_topology<boost::minstd_rand> RectangleTopologyType;

  typedef boost::adjacency_list<boost::vecS,
                                boost::vecS,
                                boost::undirectedS,
                                boost::property< boost::vertex_data_t, RectangleTopologyType::point_type >
                                > Graph;

  Graph g;

  typedef boost::graph_traits<Graph>::vertex_descriptor VertexType;

    //ReaK::pp::dvp_tree<Key, Topology, PositionMap> tree;
  typedef ReaK::pp::dvp_tree<VertexType, RectangleTopologyType, boost::property_map<Graph, boost::vertex_data_t>::type > TreeType;

  RectangleTopologyType myTopology(0,0,100,100);
  boost::property_map<Graph, boost::vertex_data_t>::type positionMap;
  TreeType tree(g, myTopology, positionMap);

  return 0;
}
