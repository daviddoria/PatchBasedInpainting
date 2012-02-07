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
template <typename VertexDescriptor, typename NearestNeighborFinderType1, typename NearestNeighborFinderType2>
struct TwoStepNearestNeighbor :
{
  NearestNeighborFinderType1 NearestNeighborFinder1;
  NearestNeighborFinderType2 NearestNeighborFinder2;

  /** The number of nearest neighbors to use from the first step of the search. */
  unsigned int K;

  /**
    * Constructor.
    * \param nearestNeighborFinder1 The functor to do the K-NN first step of the search.
    * \param nearestNeighborFinder2 The functor to do the 1-NN second step of the search.
    */
  TwoStepNearestNeighbor(NearestNeighborFinderType1 nearestNeighborFinder1, NearestNeighborFinderType2 nearestNeighborFinder2, unsigned int k_in = 1000) :
  NearestNeighborFinder1(nearestNeighborFinder1), NearestNeighborFinder2(nearestNeighborFinder2), K(k_in)
  { };

  VertexDescriptor operator()(VertexDescriptor queryNode)
  {
    // Step 1 - K-NN search on first topology
    std::multimap<float, VertexDescriptor> outputMap;
    this->NearestNeighborFinder1->find_nearest(queryNode, outputMap, this->K);

    // Step 2 - 1-NN search on result of first search, on second topology
    VertexDescriptor nearestNeighbor = this->NearestNeighborFinder2(queryPoint);
LinearSearchKNN(descriptors.begin(), descriptors.end(), kNeighbors, differenceObject, numberOfNeighbors);
  }
};

#endif
