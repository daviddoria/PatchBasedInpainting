/**
 * \file TwoStepNearestNeighbor.hpp
 * 
 * \author David Doria <daviddoria@gmail.com>
 * \date January 2012
 */

#ifndef TwoStepNearestNeighbor_HPP
#define TwoStepNearestNeighbor_HPP

#include "Visitors/NearestNeighborsDefaultVisitor.hpp"

#include <stdexcept>

/**
  * This functor template performs a KNN search, and then a best-search on the resulting K neighbors.
  *
  * \tparam VertexDescriptor The type of the vertex descriptor.
  * \tparam NeighborFinderKNN The functor that can find K-nearest neighbors.
  * \tparam NeighborFinderBest The functor that can find the best neighbor.
  */
template <typename MultipleNeighborFinderType, typename NearestNeighborFinderType, typename VisitorType = NearestNeighborsDefaultVisitor>
struct TwoStepNearestNeighbor
{
  MultipleNeighborFinderType MultipleNeighborFinder;
  NearestNeighborFinderType NearestNeighborFinder;
  VisitorType Visitor;

  /**
    * Constructor.
    * \param NeighborFinderKNN The functor to do the K-NN first step of the search.
    * \param NeighborFinderBest The functor to do the 1-NN second step of the search.
    */
  TwoStepNearestNeighbor(MultipleNeighborFinderType multipleNeighborFinder, NearestNeighborFinderType nearestNeighborFinder,
                         VisitorType visitor = VisitorType()) :
  MultipleNeighborFinder(multipleNeighborFinder), NearestNeighborFinder(nearestNeighborFinder), Visitor(visitor)
  { };

  template <typename TIterator>
  typename TIterator::value_type operator()(TIterator first, TIterator last, typename TIterator::value_type queryNode)
  {
    typedef typename TIterator::value_type VertexDescriptor;

    // Step 1 - K-NN search on first topology
    std::vector<VertexDescriptor> outputContainer;
    this->MultipleNeighborFinder(first, last, queryNode, outputContainer);
    
    std::cout << "There are " << outputContainer.size() << " items to search in the second step." << std::endl;
    if(outputContainer.size() <= 0)
      {
      throw std::runtime_error("MultipleNeighborFinder did not find any neighbors!");
      }

    Visitor.FoundNeighbors(outputContainer);

    // Step 2 - 1-NN search on result of first search, on second topology
    VertexDescriptor nearestNeighbor = this->NearestNeighborFinder(outputContainer.begin(), outputContainer.end(), queryNode);
    return nearestNeighbor;
  }

};

#endif
