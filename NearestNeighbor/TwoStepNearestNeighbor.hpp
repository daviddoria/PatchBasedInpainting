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
template <typename VertexDescriptor, typename NeighborFinderKNNType, typename NearestNeighborBestType>
struct TwoStepNearestNeighbor
{
  NeighborFinderKNNType NeighborFinderKNN;
  NearestNeighborBestType NeighborFinderBest;


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
    this->NeighborFinderKNN(queryNode, outputMap, this->K);

    // Step 2 - 1-NN search on result of first search, on second topology
    VertexDescriptor nearestNeighbor = this->NearestNeighborFinder2(queryPoint);
    NeighborFinderLinear(outputMap.begin(), outputMap.end(), differenceObject, numberOfNeighbors);
  }

  void SetK(const unsigned int k)
  {
    this->K = k;
  }
  
  unsigned int GetK() const
  {
    return this->K;
  }
private:

  /** The number of nearest neighbors to use from the first step of the search. */
  unsigned int K;
};

#endif
