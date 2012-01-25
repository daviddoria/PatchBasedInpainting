/**
 * \file topological_search.hpp
 * 
 * This library contains two simple nearest-neighbor search algorithms implemented 
 * as functor templates. This library contains, in fact, three algorithms. 
 * 
 * First, a simple min_dist_linear_search algorithm is provided which is similar to std::min_element
 * except that it stores and compares the best distance value associated to the best iterator
 * to the current one (it is a simple linear search that avoid recomputation of the distance 
 * at every iteration, which would be required if std::min_element was used instead).
 * 
 * Second, a linear_neighbor_search algorithm is provided which simply does an exhaustive linear
 * search through all the vertices of a graph to find the nearest one to a given point, in a given
 * topology (as of topologies in the Boost Graph Library). This algorithms simply wraps the 
 * min_dist_linear_search with the required distance and comparison function.
 * 
 * Third, a best_only_neighbor_search algorithm is provided which is an approximation to an 
 * exhaustive linear search by picking a number of random vertices from the graph and performing
 * a best only search down the graph to find the "nearest-neighbor". This is, of course, not going
 * to find the nearest-neighbor, but can significantly cut down on query time if finding the 
 * nearest neighbor is not a strict requirement in the algorithm.
 * 
 * \author Sven Mikael Persson <mikael.s.persson@gmail.com>
 * \date February 2011
 */

/*
 *    Copyright 2011 Sven Mikael Persson
 *
 *    THIS SOFTWARE IS DISTRIBUTED UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE v3 (GPLv3).
 *
 *    This file is part of ReaK.
 *
 *    ReaK is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    ReaK is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with ReaK (as LICENSE in the root folder).  
 *    If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef REAK_TOPOLOGICAL_SEARCH_HPP
#define REAK_TOPOLOGICAL_SEARCH_HPP

#include <boost/bind.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topology.hpp>
#include <boost/graph/properties.hpp>

#include <vector>
#include <algorithm>

  /**
   * This function template is similar to std::min_element but can be used when the comparison 
   * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the 
   * the element in the range [first,last) which has the "smallest" distance (of course, both the 
   * distance metric and comparison can be overriden to perform something other than the canonical
   * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
   * \tparam DistanceValue The value-type for the distance measures.
   * \tparam ForwardIterator The forward-iterator type.
   * \tparam GetDistanceFunction The functor type to compute the distance measure.
   * \tparam CompareFunction The functor type that can compare two distance measures (strict weak-ordering).
   * \param first Start of the range in which to search.
   * \param last One element past the last element in the range in which to search.
   * \param distance A callable object that returns a DistanceValue for a given element from the ForwardIterator dereferencing.
   * \param compare A callable object that returns true if the first element is the preferred one (less-than) of the two.
   * \param inf A DistanceValue which represents infinity (i.e. the very worst value with which to initialize the search).
   * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all the elements in the range with respect to the distance metric).
   */
  template <typename DistanceValue,
	    typename ForwardIterator,
            typename GetDistanceFunction,
	    typename CompareFunction>
  inline ForwardIterator min_dist_linear_search(ForwardIterator first,
						ForwardIterator last,
						GetDistanceFunction distance,
						CompareFunction compare,
						DistanceValue inf = std::numeric_limits<DistanceValue>::infinity()) {
    if(first == last) return last;
    DistanceValue d_best = inf;
    ForwardIterator result = last;
    for(; first != last; ++first) {
      DistanceValue d = distance(*first);
      if(compare(d, d_best)) {
	d_best = d;
	result = first;
      };
    };
    return result;
  };
  
  
  /**
   * This function template is a specialization of min_dist_linear_search for the default comparison 
   * function which is the less-than operator.
   * \tparam DistanceValue The value-type for the distance measures.
   * \tparam ForwardIterator The forward-iterator type.
   * \tparam GetDistanceFunction The functor type to compute the distance measure.
   * \param first Start of the range in which to search.
   * \param last One element past the last element in the range in which to search.
   * \param distance A callable object that returns a DistanceValue for a given element from the ForwardIterator dereferencing.
   * \param inf A DistanceValue which represents infinity (i.e. the very worst value with which to initialize the search).
   * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all the elements in the range with respect to the distance metric).
   */
  template <typename DistanceValue, typename ForwardIterator, typename GetDistanceFunction>
  inline ForwardIterator min_dist_linear_search(ForwardIterator first,
						ForwardIterator last,
						GetDistanceFunction distance,
						DistanceValue inf = std::numeric_limits<DistanceValue>::infinity()) {
    return min_dist_linear_search(first,last,distance,std::less<DistanceValue>(),inf);
  };
  
  
  /**
   * This function template is similar to std::min_element but can be used when the comparison 
   * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the 
   * the elements in the range [first,last) with the "smallest" distances (of course, both the 
   * distance metric and comparison can be overriden to perform something other than the canonical
   * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
   * This function will fill the output container with a number of nearest-neighbors.
   * \tparam DistanceValue The value-type for the distance measures.
   * \tparam ForwardIterator The forward-iterator type.
   * \tparam OutputContainer The container type which can contain the list of nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
   * \tparam GetDistanceFunction The functor type to compute the distance measure.
   * \tparam CompareFunction The functor type that can compare two distance measures (strict weak-ordering).
   * \param first Start of the range in which to search.
   * \param last One element past the last element in the range in which to search.
   * \param output The container that will have the sorted list of elements with the smallest distance.
   * \param distance A callable object that returns a DistanceValue for a given element from the ForwardIterator dereferencing.
   * \param compare A callable object that returns true if the first element is the preferred one (less-than) of the two.
   * \param max_neighbors The maximum number of elements of smallest distance to output in the sorted list.
   * \param radius The maximum distance value for which an element qualifies to be part of the output list.
   */
  template <typename DistanceValue,
	    typename ForwardIterator,
	    typename OutputContainer,
            typename GetDistanceFunction,
	    typename CompareFunction>
  inline void min_dist_linear_search(ForwardIterator first,
				     ForwardIterator last,
				     OutputContainer& output,
				     GetDistanceFunction distance,
				     CompareFunction compare,
				     unsigned int max_neighbors = 1,
				     DistanceValue radius = std::numeric_limits<DistanceValue>::infinity()) {
    output.clear();
    if(first == last) return;
    std::vector<DistanceValue> output_dist;
    for(; first != last; ++first) {
      DistanceValue d = distance(*first);
      if(!compare(d, radius)) 
	continue;
      typename std::vector<DistanceValue>::iterator it_lo = std::lower_bound(output_dist.begin(),output_dist.end(),d,compare);
      if((it_lo != output_dist.end()) || (output_dist.size() < max_neighbors)) {
	output_dist.insert(it_lo, d);
	typename OutputContainer::iterator itv = output.begin();
	for(typename std::vector<DistanceValue>::iterator it = output_dist.begin(); (itv != output.end()) && (it != it_lo); ++itv,++it) ;
	output.insert(itv, *first);
	if(output.size() > max_neighbors) {
	  output.pop_back();
	  output_dist.pop_back();
	};
      };
    };
  };
  
  /**
   * This function template is similar to std::min_element but can be used when the comparison 
   * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the 
   * the element in the range [first,last) which has the "smallest" distance (of course, both the 
   * distance metric and comparison can be overriden to perform something other than the canonical
   * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
   * \tparam DistanceValue The value-type for the distance measures.
   * \tparam ForwardIterator The forward-iterator type.
   * \tparam OutputContainer The container type which can contain the list of nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
   * \tparam GetDistanceFunction The functor type to compute the distance measure.
   * \param first Start of the range in which to search.
   * \param last One element past the last element in the range in which to search.
   * \param output The container that will have the sorted list of elements with the smallest distance.
   * \param distance A callable object that returns a DistanceValue for a given element from the ForwardIterator dereferencing.
   * \param max_neighbors The maximum number of elements of smallest distance to output in the sorted list.
   * \param radius The maximum distance value for which an element qualifies to be part of the output list.
   */
  template <typename DistanceValue,
	    typename ForwardIterator,
	    typename OutputContainer,
            typename GetDistanceFunction>
  inline void min_dist_linear_search(ForwardIterator first,
				     ForwardIterator last,
				     OutputContainer& output,
				     GetDistanceFunction distance,
				     unsigned int max_neighbors = 1,
				     DistanceValue radius = std::numeric_limits<DistanceValue>::infinity()) {
    min_dist_linear_search(first,last,output,distance,std::less<DistanceValue>(),max_neighbors,radius);
  };


  /**
   * This functor template performs a linear nearest-neighbor search through a graph by invoquing 
   * the distance function of an underlying topology. The call operator will return the vertex
   * of the graph whose position value is closest to a given position value.
   * \tparam CompareFunction The functor type that can compare two distance measures (strict weak-ordering).
   */
  template <typename CompareFunction = std::less<double> >
  struct linear_neighbor_search {

    CompareFunction m_compare;
    /**
     * Default constructor.
     * \param compare The comparison functor for ordering the distances (strict weak ordering).
     */
    linear_neighbor_search(CompareFunction compare = CompareFunction()) : m_compare(compare) { };

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
    template <typename Vertex, typename Topology, typename PositionMap>
    double distance(const typename boost::property_traits<PositionMap>::value_type& p,
                    Vertex u, const Topology& space, PositionMap position) const {
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
    template <typename Graph, typename Topology, typename PositionMap>
    typename boost::graph_traits<Graph>::vertex_descriptor operator()(const typename boost::property_traits<PositionMap>::value_type& p, 
								      Graph& g, 
								      const Topology& space, 
								      PositionMap position) {
      typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename boost::graph_traits<Graph>::vertex_iterator VertexIter;
      VertexIter ui,ui_end; tie(ui,ui_end) = vertices(g);
      return *(min_dist_linear_search(ui,ui_end,boost::bind(&linear_neighbor_search::distance<Vertex,Topology,PositionMap>,this,p,_1,space,position),m_compare,std::numeric_limits<double>::infinity()));
    };
    
    /**
     * This call-operator finds the nearest vertices of a graph, to a given position.
     * \tparam Graph The graph type which can contain the vertices, should 
     *         model boost::VertexListGraphConcept.
     * \tparam Topology The topology type which contains the positions.
     * \tparam PositionMap The property-map type which can store the position associated 
     *         with each vertex.
     * \tparam OutputContainer The container type which can contain the list of 
     *         nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
     * \param p A position in the space, to which the nearest-neighbors are sought.
     * \param output The container for the list of nearest-neighbors, the output of this 
     *        function, and will be sorted from the nearest neighbor in increasing order.
     * \param g A graph containing the vertices from which to find the nearest-neighbors.
     * \param space The topology objects which define the space in which the positions reside.
     * \param position The property-map which can retrieve the position associated to each vertex.
     * \param max_neighbors The maximum number of neighbors to have in the list.
     * \param radius The minimum distance around the position that a vertex should be in to be 
     *        considered a neighbor.
     */
    template <typename Graph, typename Topology, typename PositionMap, typename OutputContainer>
    void operator()(const typename boost::property_traits<PositionMap>::value_type& p, 
		    OutputContainer& output, 
                    Graph& g, 
		    const Topology& space, 
		    PositionMap position, 
		    unsigned int max_neighbors = 1, 
		    double radius = std::numeric_limits<double>::infinity()) {
      typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename boost::graph_traits<Graph>::vertex_iterator VertexIter;
      VertexIter ui,ui_end; tie(ui,ui_end) = boost::vertices(g);
      min_dist_linear_search(ui,ui_end,output,boost::bind(&linear_neighbor_search::distance<Vertex,Topology,PositionMap>,this,p,_1,space,position),m_compare,max_neighbors,radius);
    };
  };


  /**
   * This functor template performs a best-only nearest-neighbor search through a tree by invoquing 
   * the distance function of an underlying topology. The call operator will return the vertex
   * of the graph whose position value is likely to be closest to a given position value. This 
   * algorithm is approximate. It will select a M vertices from the graph from which it starts 
   * a best-only search, where M is obtained as M = number_of_vertices / m_vertex_num_divider.
   * \tparam CompareFunction The functor type that can compare two distance measures (strict weak-ordering).
   */
  template <typename CompareFunction = std::less<double> >
  struct best_only_neighbor_search {

    unsigned int m_vertex_num_divider;
    CompareFunction m_compare;
    /**
     * Default constructor.
     * \param aVertexNumDivider The division factor (should be greater than 1) which determines the 
     *        fraction of the total number of vertices that is used to stem the best-only searches. 
     *        Typical values are between 4 and 10.
     * \param compare The comparison functor for ordering the distances (strict weak ordering).
     */
    best_only_neighbor_search(unsigned int aVertexNumDivider = 10, 
			      CompareFunction compare = CompareFunction()) : 
                              m_vertex_num_divider(aVertexNumDivider), m_compare(compare) { };

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
    template <typename Vertex, typename Topology, typename PositionMap>
    double distance(const typename boost::property_traits<PositionMap>::value_type& p,
                    Vertex u, const Topology& space, PositionMap position) const {
      return space.distance(p, get(position, u));
    };

    template <typename Graph, typename Topology, typename PositionMap>
    void search(const typename boost::property_traits<PositionMap>::value_type& p, 
		typename boost::graph_traits<Graph>::vertex_descriptor& u, 
		double& d_min, Graph& g, const Topology& space, PositionMap position) {
      typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename boost::graph_traits<Graph>::out_edge_iterator EdgeIter;
      d_min = distance(p,u,space,position); 
      while(boost::out_degree(u,g)) {
        Vertex v_min = u;
        EdgeIter ei, ei_end;
        for(boost::tie(ei,ei_end) = boost::out_edges(u,g); ei != ei_end; ++ei) {
          Vertex v = boost::target(*ei,g); double d_v = distance(p,v,space,position); 
          if(m_compare(d_v,d_min)) {
            d_min = d_v; v_min = v;
          };
        };
        if(v_min == u)
          return;
        u = v_min;
      };
      return;
    };
    
    /**
     * This call-operator finds the nearest vertex of a graph, to a given position.
     * \tparam Graph The graph type which can contain the vertices, should 
     *         model boost::VertexListGraphConcept and boost::IncidenceGraphConcept.
     * \tparam Topology The topology type which contains the positions.
     * \tparam PositionMap The property-map type which can store the position associated with each vertex.
     * \param p A position in the space, to which the nearest-neighbor is sought.
     * \param g A graph containing the vertices from which to find the nearest-neighbor, 
     *        should be tree-structured.
     * \param space The topology objects which define the space in which the positions reside.
     * \param position The property-map which can retrieve the position associated to each vertex.
     */
    template <typename Graph, typename Topology, typename PositionMap>
    typename boost::graph_traits<Graph>::vertex_descriptor operator()(const typename boost::property_traits<PositionMap>::value_type& p, 
								      Graph& g, const Topology& space, PositionMap position) {
      typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
      if(m_vertex_num_divider == 0)
	m_vertex_num_divider = 1;
      Vertex u_min = boost::vertex(std::rand() % boost::num_vertices(g),g);
      double d_min;
      search(p,u_min,d_min,g,space,position);
      for(unsigned int i = 0; i < boost::num_vertices(g) / m_vertex_num_divider; ++i) {
        double d_v; Vertex v = boost::vertex(std::rand() % boost::num_vertices(g),g);
        search(p,v,d_v,g,space,position);
        if(m_compare(d_v,d_min)) {
          d_min = d_v; u_min = v;
        };
      };
      return u_min;
    };
    
    
    template <typename Graph, typename Topology, typename PositionMap, typename OutputContainer>
    void search(const typename boost::property_traits<PositionMap>::value_type& p, 
		typename boost::graph_traits<Graph>::vertex_descriptor u, OutputContainer& output, std::vector<double>& output_dist,
		double d_min, Graph& g, const Topology& space, PositionMap position, unsigned int max_neighbors, double radius) {
      typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename boost::graph_traits<Graph>::out_edge_iterator EdgeIter;
      if(m_compare(d_min, radius)) {
        std::vector<double>::iterator it_lo = std::lower_bound(output_dist.begin(),output_dist.end(),d_min,m_compare);
        if((it_lo != output_dist.end()) || (output_dist.size() < max_neighbors)) {
 	  output_dist.insert(it_lo, d_min);
	  typename OutputContainer::iterator itv = output.begin();
	  for(std::vector<double>::iterator it = output_dist.begin(); (itv != output.end()) && (it != it_lo); ++itv,++it) ;
	  output.insert(itv, u);
	  if(output.size() > max_neighbors) {
	    output.pop_back();
	    output_dist.pop_back();
	  };
        };
      };
      EdgeIter ei, ei_end;
      for(boost::tie(ei,ei_end) = boost::out_edges(u,g); ei != ei_end; ++ei) {
	Vertex v = boost::target(*ei,g); double d_v = distance(p,v,space,position);
	if(m_compare(d_v,d_min))
	  search(p,v,output,output_dist,d_v,g,space,position,max_neighbors,radius);
      };
    };
    
    /**
     * This call-operator finds the nearest vertices of a graph, to a given position.
     * \tparam Graph The graph type which can contain the vertices, should 
     *         model boost::VertexListGraphConcept and boost::IncidenceGraphConcept.
     * \tparam Topology The topology type which contains the positions.
     * \tparam PositionMap The property-map type which can store the position associated 
     *         with each vertex.
     * \tparam OutputContainer The container type which can contain the list of 
     *         nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
     * \param p A position in the space, to which the nearest-neighbors are sought.
     * \param output The container for the list of nearest-neighbors, the output of this 
     *        function, and will be sorted from the nearest neighbor in increasing order.
     * \param g A graph containing the vertices from which to find the nearest-neighbors, 
     *        should be tree-structured.
     * \param space The topology objects which define the space in which the positions reside.
     * \param position The property-map which can retrieve the position associated to each vertex.
     * \param max_neighbors The maximum number of neighbors to have in the list.
     * \param radius The minimum distance around the position that a vertex should be in to be 
     *        considered a neighbor.
     */
    template <typename Graph, typename Topology, typename PositionMap, typename OutputContainer>
    void operator()(const typename boost::property_traits<PositionMap>::value_type& p, OutputContainer& output, 
		    Graph& g, const Topology& space, PositionMap position, unsigned int max_neighbors = 1, double radius = std::numeric_limits<double>::infinity()) {
      typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
      output.clear();
      std::vector<double> output_dist;
      if(m_vertex_num_divider == 0)
	m_vertex_num_divider = 1;
      for(unsigned int i = 0; i < boost::num_vertices(g) / m_vertex_num_divider; ++i) {
        Vertex v = boost::vertex(std::rand() % boost::num_vertices(g),g);
	double d_v = distance(p,v,space,position);
        search(p,v,output,output_dist,d_v,g,space,position,max_neighbors,radius);
      };
    };
  };

#endif
