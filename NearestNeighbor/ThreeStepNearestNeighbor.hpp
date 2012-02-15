/**
 * \file TwoStepNearestNeighbor.hpp
 * 
 * \author David Doria <daviddoria@gmail.com>
 * \date January 2012
 */

#ifndef ThreeStepNearestNeighbor_HPP
#define ThreeStepNearestNeighbor_HPP

#include <iostream>
#include <stdexcept>

/**
  * This functor template performs a KNN search, and then a best-search on the resulting K neighbors.
  *
  * \tparam VertexDescriptor The type of the vertex descriptor.
  * \tparam NeighborFinderKNN The functor that can find K-nearest neighbors.
  * \tparam NeighborFinderBest The functor that can find the best neighbor.
  */
template <typename MultipleNeighborFinderType1, typename MultipleNeighborFinderType2, typename NearestNeighborFinderType>
struct ThreeStepNearestNeighbor
{
  MultipleNeighborFinderType1 MultipleNeighborFinder1;
  MultipleNeighborFinderType2 MultipleNeighborFinder2;
  NearestNeighborFinderType NearestNeighborFinder;

  /**
    * Constructor.
    * \param MultipleNeighborFinderType1 The functor to do the K-NN first step of the search.
    * \param MultipleNeighborFinderType2 The functor to do the K-NN second step of the search.
    * \param NeighborFinderBest The functor to do the 1-NN last step of the search.
    */
  ThreeStepNearestNeighbor(MultipleNeighborFinderType1 multipleNeighborFinder1, MultipleNeighborFinderType2 multipleNeighborFinder2, NearestNeighborFinderType nearestNeighborFinder) :
  MultipleNeighborFinder1(multipleNeighborFinder1), MultipleNeighborFinder2(multipleNeighborFinder2), NearestNeighborFinder(nearestNeighborFinder)
  { };

  template <typename TIterator>
  typename TIterator::value_type operator()(TIterator first, TIterator last, typename TIterator::value_type queryNode)
  {
    typedef typename TIterator::value_type VertexDescriptor;

    // Step 1 - K-NN search on first topology
    std::vector<VertexDescriptor> outputContainer1;
    this->MultipleNeighborFinder1(first, last, queryNode, outputContainer1);

    std::cout << "There are " << outputContainer1.size() << " items to search in the second step." << std::endl;
    if(outputContainer1.size() <= 0)
      {
      throw std::runtime_error("MultipleNeighborFinder did not find any neighbors!");
      }

    // Step 1 - K-NN search on second topology
    std::vector<VertexDescriptor> outputContainer2;
    this->MultipleNeighborFinder1(outputContainer1.begin(), outputContainer1.end(), queryNode, outputContainer2);

    std::cout << "There are " << outputContainer2.size() << " items to search in the third step." << std::endl;
    if(outputContainer2.size() <= 0)
      {
      throw std::runtime_error("MultipleNeighborFinder did not find any neighbors!");
      }

    // Step 2 - 1-NN search on result of first search, on second topology
    VertexDescriptor nearestNeighbor = this->NearestNeighborFinder(outputContainer2.begin(), outputContainer2.end(), queryNode);
    return nearestNeighbor;
  }

};

#endif
