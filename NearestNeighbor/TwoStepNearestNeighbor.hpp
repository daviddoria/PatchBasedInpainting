/**
 * \file TwoStepNearestNeighbor.hpp
 * 
 * \author David Doria <daviddoria@gmail.com>
 * \date January 2012
 */

#ifndef TwoStepNearestNeighbor_HPP
#define TwoStepNearestNeighbor_HPP
/**
  * This functor template performs a KNN search, and then a best-search on the resulting K neighbors.
  *
  * \tparam VertexDescriptor The type of the vertex descriptor.
  * \tparam NeighborFinderKNN The functor that can find K-nearest neighbors.
  * \tparam NeighborFinderBest The functor that can find the best neighbor.
  */
template <typename NeighborFinderKNNType, typename NearestNeighborBestType>
struct TwoStepNearestNeighbor
{
  NeighborFinderKNNType NeighborFinderKNN;
  NearestNeighborBestType NeighborFinderBest;

  /**
    * Constructor.
    * \param NeighborFinderKNN The functor to do the K-NN first step of the search.
    * \param NeighborFinderBest The functor to do the 1-NN second step of the search.
    */
  TwoStepNearestNeighbor(NeighborFinderKNNType neighborFinderKNN, NearestNeighborBestType neighborFinderBest) :
  NeighborFinderKNN(neighborFinderKNN), NeighborFinderBest(neighborFinderBest)
  { };

  template <typename TIterator>
  typename TIterator::value_type operator()(TIterator first, TIterator last, typename TIterator::value_type queryNode)
  {
    typedef typename TIterator::value_type VertexDescriptor;

    // Step 1 - K-NN search on first topology
    std::vector<VertexDescriptor> outputMap;
    this->NeighborFinderKNN(first, last, queryNode, outputMap);

    // Step 2 - 1-NN search on result of first search, on second topology
    VertexDescriptor nearestNeighbor = this->NeighborFinderBest(outputMap.begin(), outputMap.end(), queryNode);
    return nearestNeighbor;
  }

};

#endif
