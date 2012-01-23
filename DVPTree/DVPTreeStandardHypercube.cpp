#include <iostream>

#include "metric_space_search.hpp"

namespace boost {
  enum vertex_data_t { vertex_data };

  BOOST_INSTALL_PROPERTY(vertex, data);
};

int main(int argc, char *argv[])
{
  const unsigned int dimension = 6;
  typedef boost::hypercube_topology<dimension, boost::minstd_rand> TopologyType;

  typedef boost::adjacency_list<boost::vecS,
                                boost::vecS,
                                boost::undirectedS,
                                boost::property< boost::vertex_data_t, TopologyType::point_type >
                                > Graph;

  Graph g;

  typedef boost::graph_traits<Graph>::vertex_descriptor VertexType;

  boost::property_map<Graph, boost::vertex_data_t>::type positionMap = get(boost::vertex_data, g); // You forgot to actually initialize the positionMap! (which was the source of the seg-fault on first use)

  TopologyType myTopology;

  typedef TopologyType::point_type PointType;

  typedef ReaK::pp::dvp_tree<VertexType, TopologyType, boost::property_map<Graph, boost::vertex_data_t>::type > TreeType;


  // Add vertices to the graph and corresponding points increasin integer points to the tree.
  // The experiment here is to query the nearest neighbor of a point like (5.2, 5.2, 5.1, 5.3, 5.2, 5.1)
  // and ensure we get back (5,5,5,5,5,5)
  unsigned int numberOfVertices = 100;
  for(unsigned int vertexId = 0; vertexId < numberOfVertices; ++vertexId)
  {
    VertexType v = boost::add_vertex(g);
    PointType p;
    for(unsigned int dim = 0; dim < dimension; ++dim)
      {
      p[dim] = vertexId;
      }
    boost::put(positionMap, v, p); // segfault on this line
  };

  TreeType tree(g, myTopology, positionMap); // Prefer to initialize the DVP-tree with a filled graph, this way, the entire DVP-tree will be initialized at once (gets best results).

  ReaK::pp::multi_dvp_tree_search<Graph, TreeType> nearestNeighborFinder;
  nearestNeighborFinder.graph_tree_map[&g] = &tree;

  PointType queryPoint;
  for(unsigned int dim = 0; dim < dimension; ++dim)
    {
    queryPoint[dim] = 5.2;
    }

  VertexType nearestNeighbor = nearestNeighborFinder(queryPoint, g, myTopology, positionMap);
  std::cout << "nearestNeighbor: " << nearestNeighbor << std::endl;

  return 0;
}
