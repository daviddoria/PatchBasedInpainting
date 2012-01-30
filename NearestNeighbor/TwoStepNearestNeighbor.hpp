/**
 * \file TwoStepNearestNeighbor.hpp
 * 
 * \author David Doria <daviddoria@gmail.com>
 * \date January 2012
 */

#ifndef TwoStepNearestNeighbor_HPP
#define TwoStepNearestNeighbor_HPP
/**
   * This functor template performs a linear nearest-neighbor search through a graph by invoquing 
   * the distance function of an underlying topology. The call operator will return the vertex
   * of the graph whose position value is closest to a given position value.
   * \tparam CompareFunction The functor type that can compare two distance measures (strict weak-ordering).
   */
  template <typename Topology1, typename Topology2, typename CompareFunction = std::less<double> >
  struct TwoStepNearestNeighbor
  {

    CompareFunction m_compare;
    /**
     * Default constructor.
     * \param compare The comparison functor for ordering the distances (strict weak ordering).
     */
    TwoStepNearestNeighbor(CompareFunction compare = CompareFunction()) : m_compare(compare) { };

    /**
     * This function template computes the topological distance between a position and the position of a
     * vertex of a graph. This function is used as a helper to the call-operator overloads.
     * \tparam Vertex The vertex descriptor type.
     * \tparam Topology The topology type which contains the positions.
     * \tparam PositionMap The property-map type which can store the position associated with each vertex.
     * \param p A position in the space.
     * \param u A vertex which has a position associated to it, via the position property-map.
     * \param space The topology objects which define the space in which the positions reside.
     * \param position The property-map which can retrieve the position associated to each vertex.
     */
    template <typename Vertex, typename Topology1, typename PositionMap1, typename Topology2, typename PositionMap2>
    double distance(const typename boost::property_traits<PositionMap1>::value_type& p_space1, const typename boost::property_traits<PositionMap2>::value_type& p_space2,
                    Vertex u, const Topology1& space1, PositionMap1 positionMap1, const Topology2& space2, PositionMap2 positionMap2) const 
    {
      return space.distance(p, get(position, u));
    };

    /**
     * This call-operator finds the nearest vertex of a graph, to a given position.
     * \tparam Graph The graph type which can contain the vertices, should model boost::VertexListGraphConcept.
     * \tparam Topology The topology type which contains the positions.
     * \tparam PositionMap The property-map type which can store the position associated with each vertex.
     * \param p A position in the space, to which the nearest-neighbor is sought.
     * \param g A graph containing the vertices from which to find the nearest-neighbor.
     * \param space The topology objects which define the space in which the positions reside.
     * \param position The property-map which can retrieve the position associated to each vertex.
     */
    template <typename Graph, typename Topology1, typename PositionMap1, typename Topology2, typename PositionMap2>
    typename boost::graph_traits<Graph>::vertex_descriptor operator()(const typename boost::property_traits<PositionMap>::value_type& p, 
                                                                      Graph& g, 
                                                                      const Topology& space, 
                                                                      PositionMap position) {
      typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename boost::graph_traits<Graph>::vertex_iterator VertexIter;
      VertexIter ui,ui_end; tie(ui,ui_end) = vertices(g);
      return *(min_dist_linear_search(ui,ui_end,
                                      boost::bind(&TwoStepNearestNeighbor::distance<Vertex,Topology,PositionMap>,this,p,_1,space,position),
                                      m_compare,std::numeric_limits<double>::infinity()));
    };

#endif
