#include <iostream>

#include "metric_space_search.hpp"

struct CustomType
{
  
};

class CustomTopology
{
 public:
  typedef CustomType point_type;
  typedef float point_difference_type;

  typedef boost::uniform_01<std::minstd_rand, double> rand_t;

  explicit CustomTopology()
    : gen_ptr(new std::minstd_rand), rand(new rand_t(*gen_ptr))
  { }

  point_type random_point() const 
  {
    point_type p;
//     for (std::size_t i = 0; i < Dims; ++i)
//       p[i] = (*rand)() * scaling;
    return p;
  }

  point_type bound(point_type a) const
  {
//     BOOST_USING_STD_MIN();
//     BOOST_USING_STD_MAX();
    point_type p;
//     for (std::size_t i = 0; i < Dims; ++i)
//       p[i] = min BOOST_PREVENT_MACRO_SUBSTITUTION (scaling, max BOOST_PREVENT_MACRO_SUBSTITUTION (-scaling, a[i]));
    return p;
  }

  double distance_from_boundary(point_type a) const
  {
//     BOOST_USING_STD_MIN();
//     BOOST_USING_STD_MAX();
// #ifndef BOOST_NO_STDC_NAMESPACE
//     using std::abs;
// #endif
//     BOOST_STATIC_ASSERT (Dims >= 1);
//     double dist = abs(scaling - a[0]);
//     for (std::size_t i = 1; i < Dims; ++i)
//       dist = min BOOST_PREVENT_MACRO_SUBSTITUTION (dist, abs(scaling - a[i]));
    float dist = 0;
    return dist;
  }

  point_type center() const {
    point_type result;
//     for (std::size_t i = 0; i < Dims; ++i)
//       result[i] = scaling * .5;
    return result;
  }

  point_type origin() const {
    point_type result;
//     for (std::size_t i = 0; i < Dims; ++i)
//       result[i] = 0;
    return result;
  }

  point_difference_type extent() const {
    point_difference_type result;
//     for (std::size_t i = 0; i < Dims; ++i)
//       result[i] = scaling;
    return result;
  }

  double distance(point_type a, point_type b) const
  {
    double dist = 0.0f;
    return dist;
  }
  
 private:
  std::shared_ptr<std::minstd_rand> gen_ptr;
  std::shared_ptr<rand_t> rand;
  double scaling;
};

namespace boost {
  enum vertex_data_t { vertex_data };

  BOOST_INSTALL_PROPERTY(vertex, data);
};

int main(int argc, char *argv[])
{
  const unsigned int dimension = 6;
  //typedef boost::hypercube_topology<dimension, boost::minstd_rand> TopologyType;
  // This distance(a,b) function is the equivialent of matlab's norm(a-b)

  typedef boost::adjacency_list<boost::vecS,
                                boost::vecS,
                                boost::undirectedS,
                                boost::property< boost::vertex_data_t, CustomTopology::point_type >
                                > Graph;

  Graph g;

  typedef boost::graph_traits<Graph>::vertex_descriptor VertexType;

  boost::property_map<Graph, boost::vertex_data_t>::type positionMap = get(boost::vertex_data, g);

  CustomTopology myTopology;
  typedef CustomTopology::point_type PointType;

  typedef ReaK::pp::dvp_tree<VertexType, CustomTopology, boost::property_map<Graph, boost::vertex_data_t>::type > TreeType;

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
      //p[dim] = vertexId;
      }
    boost::put(positionMap, v, p);
  };

  // Prefer to initialize the DVP-tree with a filled graph, this way, the entire DVP-tree will be initialized at once (gets best results).
  TreeType tree(g, myTopology, positionMap);

  ReaK::pp::multi_dvp_tree_search<Graph, TreeType> nearestNeighborFinder;
  nearestNeighborFinder.graph_tree_map[&g] = &tree;

  PointType queryPoint;
  for(unsigned int dim = 0; dim < dimension; ++dim)
    {
    //queryPoint[dim] = 5.2;
    }

  VertexType nearestNeighbor = nearestNeighborFinder(queryPoint, g, myTopology, positionMap);
  std::cout << "nearestNeighbor: " << nearestNeighbor << std::endl;

  return 0;
}
