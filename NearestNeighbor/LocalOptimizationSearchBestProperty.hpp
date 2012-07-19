#ifndef LocalOptimizationSearchBestProperty_HPP
#define LocalOptimizationSearchBestProperty_HPP

// STL
#include <iostream>
#include <limits>

// Custom
#include "LinearSearchKNNProperty.hpp"

// Submodules
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKHelpers.h"

/**
   * This function template is similar to std::min_element but can be used when the comparison
   * involves computing a derived quantity. This algorithm will search for the
   * the element in the range [first,last) which has the "smallest" distance (of course, both the
   * distance metric and comparison can be overriden to perform something other than the canonical
   * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
   * \tparam DistanceValueType The value-type for the distance measures.
   * \tparam DistanceFunctionType The functor type to compute the distance measure.
   * \tparam CompareFunctionType The functor type that can compare two distance measures (strict weak-ordering).
   */
template <typename PropertyMapType,
          typename DistanceFunctionType,
          typename DistanceValueType = float,
          typename CompareFunctionType = std::less<DistanceValueType> >
struct LocalOptimizationSearchBestProperty
{
  PropertyMapType PropertyMap;
  DistanceFunctionType DistanceFunction;
  CompareFunctionType CompareFunction;

  LocalOptimizationSearchBestProperty(PropertyMapType propertyMap, DistanceFunctionType distanceFunction = DistanceFunctionType(),
                           CompareFunctionType compareFunction = CompareFunctionType()) :
  PropertyMap(propertyMap), DistanceFunction(distanceFunction), CompareFunction(compareFunction){}

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  template <typename TIterator>
  typename std::iterator_traits<TIterator>::value_type operator()(TIterator first, TIterator last,
                                                                  typename std::iterator_traits<TIterator>::value_type query)
  {
    // Rather than just compare 'query' to all of the input elements (as we do in LinearSearchBestProperty),
    // here we perform a LinearSearchKNNProperty for the 'query' as well as all of its neighbors.

    // Compute the best patches for the direct query patch

    // This is the number of best matches to store for every neighbor patch and the query patch itself
    unsigned int numberOfMatchesToMaintain = 10;

    typedef LinearSearchKNNProperty<PropertyMapType,
                                   DistanceFunctionType> KNNSearchFunctorType;
    KNNSearchFunctorType knnSearchFunctor(PropertyMap, numberOfMatchesToMaintain);

    typedef std::vector<typename std::iterator_traits<TIterator>::value_type> KNNContainerType;
    KNNContainerType queryBestMatches(numberOfMatchesToMaintain);

    knnSearchFunctor(first, last, query, queryBestMatches.begin());

    std::cout << "Found top source patches for direct query patch." << std::endl;

    // Compute the neighbors of the query patch
    itk::Index<2> queryIndex = Helpers::ConvertFrom<itk::Index<2>, typename TIterator::value_type>(query);

    std::vector<itk::Index<2> > neighborPixels =
          ITKHelpers::Get8NeighborsInRegion(get(this->PropertyMap, query).GetImage()->GetLargestPossibleRegion(), queryIndex);
    std::vector<itk::ImageRegion<2> > neighborRegions(neighborPixels.size());

    unsigned int radius = get(this->PropertyMap, query).GetRegion().GetSize()[0]/2;
    for(unsigned int i = 0; i < neighborPixels.size(); ++i)
    {
      neighborRegions[i] = ITKHelpers::GetRegionInRadiusAroundPixel(neighborPixels[i],
                                                                    radius);
    }

    // Setup the storage of all neighbors KNN
    std::vector<KNNContainerType>
          allNeigbhorKNN(neighborRegions.size());
    // Resize/presize all of the containers
    for(unsigned int i = 0; i < allNeigbhorKNN.size(); ++i)
    {
      allNeigbhorKNN[i].resize(numberOfMatchesToMaintain);
    }

    // For each neighbor, compute a list of candidate filling patches
    for(unsigned int neighborId = 0; neighborId < neighborRegions.size(); ++neighborId)
    {
      KNNContainerType neighborKNNContainer(numberOfMatchesToMaintain);
      //typename KNNContainerType::iterator resultIterator = knnSearchFunctor(first, last, query, neighborKNNContainer.begin()); // output is not used
      knnSearchFunctor(first, last, query, neighborKNNContainer.begin());

      allNeigbhorKNN[neighborId] = neighborKNNContainer;
    }

    std::cout << "Found top source patches for all neighbor patches." << std::endl;

    // For each top patch of the direct query pixel, sum the minimum distance to all
    // matches to the neighbor patch

    // "the minimum differences of a candidate patch for the
    // target region with respect to all the candidate patches in
    // the neighborhood."
    typedef std::pair<float, typename std::iterator_traits<TIterator>::value_type > ScoreType;
    std::vector<ScoreType> scores(numberOfMatchesToMaintain);
    for(unsigned int i = 0; i < scores.size(); ++i)
    {
      scores[i].first = 0.0f;
    }

    for(unsigned int directPatchMatchId = 0; directPatchMatchId < numberOfMatchesToMaintain; ++directPatchMatchId)
    {
      for(unsigned int neighborId = 0; neighborId < neighborRegions.size(); ++neighborId)
      {
        float minScore = std::numeric_limits<float>::max();

        for(unsigned int neighborTopPatchId = 0; neighborTopPatchId < numberOfMatchesToMaintain; ++neighborTopPatchId)
        {
          float score = this->DistanceFunction(get(this->PropertyMap, allNeigbhorKNN[neighborId][neighborTopPatchId]),
                                               get(this->PropertyMap, queryBestMatches[directPatchMatchId]) );
          if(score < minScore)
          {
            minScore = score;
          }
        }
        scores[directPatchMatchId].first += minScore;
        scores[directPatchMatchId].second = queryBestMatches[directPatchMatchId];
      }
    }

    std::cout << "Computed summed errors." << std::endl;

    // Sort the patch data by the new scores
    std::sort(scores.begin(), scores.end(),
              [](const ScoreType& val1, const ScoreType& val2) {
                  return val1.first < val2.first; } );

    std::cout << "Finished sorting top patches by local optimization criteria." << std::endl;
    return scores[0].second;
  }
};

#endif
