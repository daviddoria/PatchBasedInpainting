#include <iostream>

#include "metric_space_search.hpp"

namespace boost {
  enum vertex_position_t { vertex_position };

  BOOST_INSTALL_PROPERTY(vertex, position);
};

int main(int argc, char *argv[])
{

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;

  //ReaK::pp::dvp_tree<Key, Topology, PositionMap> tree;

  typedef boost::graph_traits<Graph>::vertex_descriptor VertexType;
  typedef ReaK::pp::dvp_tree<VertexType, boost::rectangle_topology<>, boost::property_map<Graph, boost::vertex_position_t>::type > TreeType;
  TreeType tree;

  return 0;
}
