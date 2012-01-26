// http://www.boost.org/doc/libs/1_46_0/libs/graph/doc/topology.html

//DistanceMetricConcept
#include <cmath>

#include <iostream>

#include <boost/graph/grid_graph.hpp>

#include "NearestNeighbor/topological_search.hpp"

class custom_topology
{
public:
  typedef float point_type;
  typedef float point_difference_type;

  /**
    * Returns the distance between two points.
    */
  double distance(const point_type& a, const point_type& b) const 
  {
    return fabs(a-b);
  }

};

int main(int argc, char *argv[])
{
  typedef boost::grid_graph<2> GraphType;

  const unsigned int dimension = 5;
  boost::array<std::size_t, 2> lengths = { { dimension, dimension } };
  GraphType graph(lengths);

  typedef boost::graph_traits<GraphType>::vertex_descriptor VertexDescriptor;

  typedef custom_topology TopologyType;
  TopologyType myTopology;
  typedef TopologyType::point_type PointType;

  std::vector<PointType> vertexData(dimension * dimension);

  // This is an "exterior property" of the grid_graph
  typedef boost::property_map<GraphType, boost::vertex_index_t>::const_type IndexMapType;

  IndexMapType indexMap(get(boost::vertex_index, graph));

  typedef boost::iterator_property_map<std::vector<PointType>::iterator, IndexMapType> MapType;
  MapType myMap(vertexData.begin(), indexMap);

  typedef linear_neighbor_search<> SearchType;

  // Add integer positions to the graph vertices.
  // The experiment here is to query the nearest neighbor of a point like 5.2
  // and ensure we get back 5.
  unsigned int numberOfVertices = 10;
  for(unsigned int vertexId = 0; vertexId < numberOfVertices; ++vertexId)
  {
    VertexDescriptor v = vertex(vertexId, graph);
    std::cout << "adding " << vertexId << " to " << v[0] << " " << v[1] << std::endl;
    PointType p = vertexId;

    boost::put(myMap, v, p);
  };

  PointType queryPoint = 5.2;

  SearchType search;

  VertexDescriptor nearestNeighbor = search(queryPoint, graph, myTopology, myMap);

  std::cout << "nearestNeighbor: " << nearestNeighbor[0] << " " << nearestNeighbor[1] << std::endl;

  return 0;
}
