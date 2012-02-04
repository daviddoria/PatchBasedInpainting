#include <iostream>

#include <boost/graph/grid_graph.hpp>

#include "NearestNeighbor/metric_space_search.hpp"

int main(int argc, char *argv[])
{
  typedef boost::grid_graph<2> GraphType;

  //const unsigned int dimension = 5;
  const unsigned int gridDimension = 300;
  boost::array<std::size_t, 2> lengths = { { gridDimension, gridDimension } };
  GraphType graph(lengths);

  typedef boost::graph_traits<GraphType>::vertex_descriptor VertexDescriptor;

  VertexDescriptor v = { { 0, 1 } };

  const unsigned int descriptorDimension = 100;
  typedef boost::hypercube_topology<descriptorDimension, boost::minstd_rand> TopologyType;
  TopologyType myTopology;
  typedef TopologyType::point_type PointType;

  std::vector<PointType> vertexData(gridDimension * gridDimension);

  // This is an "exterior property" of the grid_graph
  typedef boost::property_map<GraphType, boost::vertex_index_t>::const_type IndexMapType;

  IndexMapType indexMap(get(boost::vertex_index, graph));

  typedef boost::iterator_property_map<std::vector<PointType>::iterator, IndexMapType> MapType;
  MapType myMap(vertexData.begin(), indexMap);

  // Add vertices to the graph and corresponding points increasing integer points to the tree.
  // The experiment here is to query the nearest neighbor of a point like (5.2, 5.2, 5.1, 5.3, 5.2, 5.1)
  // and ensure we get back (5,5,5,5,5,5)
  unsigned int counter = 0;
//   for(unsigned int vertexId = 0; vertexId < num_vertices(graph); ++vertexId)
//   {
//     PointType p;
//     for(unsigned int dim = 0; dim < descriptorDimension; ++dim)
//       {
//       p[dim] = vertexId;
//       }
//     boost::put(myMap, v, p);
//   };
  typedef typename boost::graph_traits<GraphType>::vertex_iterator VertexIter;
  VertexIter ui,ui_end; tie(ui,ui_end) = vertices(graph);
  for(VertexIter iter = ui; iter != ui_end; ++iter)
    {
    PointType p;
    for(unsigned int dim = 0; dim < descriptorDimension; ++dim)
      {
      p[dim] = counter;
      }
    counter++;
    boost::put(myMap, *iter, p);
    }

  // Prefer to initialize the DVP-tree with a filled graph, this way,
  // the entire DVP-tree will be initialized at once (gets best results).
  typedef dvp_tree<VertexDescriptor, TopologyType, MapType> TreeType;
  TreeType tree(graph, myTopology, myMap);

  multi_dvp_tree_search<GraphType, TreeType> nearestNeighborFinder;
  nearestNeighborFinder.graph_tree_map[&graph] = &tree;

  PointType queryPoint;
  for(unsigned int dim = 0; dim < descriptorDimension; ++dim)
    {
    queryPoint[dim] = 5.2;
    }

  VertexDescriptor nearestNeighbor = nearestNeighborFinder(queryPoint, graph, myTopology, myMap);
  std::cout << "nearestNeighbor[0]: " << nearestNeighbor[0] << std::endl;

  return 0;
}
