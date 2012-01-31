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
template <typename VertexDescriptor, typename Topology1, typename PositionMap1, typename Topology2, typename PositionMap2, typename CompareFunction = std::less<double> >
struct TwoStepNearestNeighbor
{

  CompareFunction m_compare;
  Topology1 m_topology1;
  PositionMap1 m_positionMap1;

  Topology2 m_topology2;
  PositionMap2 m_positionMap2;

  /** The number of nearest neighbors to use from the first step of the search. */
  unsigned int K;
  /**
    * Default constructor.
    * \param compare The comparison functor for ordering the distances (strict weak ordering).
    */
  TwoStepNearestNeighbor(Topology1 topology1, PositionMap1 positionMap1, Topology2 topology2, PositionMap2 positionMap2, const unsigned int k, CompareFunction compare = CompareFunction()) :
  m_compare(compare), m_topology1(topology1), m_positionMap1(positionMap1), m_topology2(topology2), m_positionMap2(positionMap2), K(k) { };

  VertexDescriptor find_nearest(VertexDescriptor v)
  {
    // Step 1 - K-NN search on first topology
    std::multimap<float, VertexDescriptor> outputMap;
    find_nearest(v, outputMap, K);
    
    // Step 2 - 1-NN search on result of first search, on second topology
  }
};

#endif
