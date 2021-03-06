#ifndef LinearSearchCriteriaProperty_HPP
#define LinearSearchCriteriaProperty_HPP

// STL
#include <iostream>
#include <limits> // for infinity()
#include <algorithm> // for lower_bound()

// ITK
#include "itkImage.h"
#include "itkImageFileWriter.h"

/**
  * This function will search for the compute the difference between a query node's property and
  * all other nodes properties, and return only those which have a difference (float) below a specified
  * threshold.
  * \tparam DistanceValue The value-type for the distance measures.
  * \tparam DistanceFunctionType The functor type to compute the distance measure.
  * \tparam CompareFunctionType The functor type that can compare two distance measures (strict weak-ordering).
  */
template <typename PropertyMapType,
          typename DistanceFunctionType,
          typename DistanceValueType = float,
          typename CompareFunctionType = std::less<DistanceValueType> >
class LinearSearchCriteriaProperty
{
  PropertyMapType PropertyMap;
  float DistanceThreshold;
  DistanceFunctionType DistanceFunction;
  CompareFunctionType CompareFunction;

public:
  /**
  * This function will search for the compute the difference between a query node's property and
  * all other nodes properties, and return only those which have a difference (float) below a specified
  * threshold.
  * \param propertyMap The property map from which to get the values to compare.
  * \param distanceThreshold The value which the distance must be less than to pass the test.
  */
  LinearSearchCriteriaProperty(PropertyMapType propertyMap, const float distanceThreshold, DistanceFunctionType distanceFunction = DistanceFunctionType(), CompareFunctionType compareFunction = CompareFunctionType()) : 
  PropertyMap(propertyMap), DistanceThreshold(distanceThreshold), DistanceFunction(distanceFunction), CompareFunction(compareFunction)
  {
    std::cout << "DistanceThreshold: " << DistanceThreshold << std::endl;
  }

  void SetDistanceThreshold(const float distanceThreshold)
  {
    this->DistanceThreshold = distanceThreshold;
  }
  
  float GetDistanceThreshold() const
  {
    return this->DistanceThreshold;
  }

  /**
  * This function will search for the compute the difference between a query node's property and
  * all other nodes properties, and return only those which have a difference (float) below a specified
  * threshold.
  * \tparam ForwardIteratorType The forward-iterator type.
  * \tparam OutputContainerType The container type which can contain the list of nearest-neighbors (STL like container, with iterators, insert, size, and pop_back).
  * \param first Start of the range in which to search.
  * \param last One element past the last element in the range in which to search.
  * \param output The container that will have the sorted list of elements with the smallest distance.
  * \param queryNode The node at which to compare the property.
  */
  template <typename ForwardIteratorType, typename OutputContainerType>
  void operator()(ForwardIteratorType first, ForwardIteratorType last, typename ForwardIteratorType::value_type queryNode, OutputContainerType& output)
  {
    output.clear();
    if(first == last)
    {
      return;
    }

    for(ForwardIteratorType iter = first; iter != last; ++iter)
    {
      DistanceValueType d = DistanceFunction(get(PropertyMap, *iter), get(PropertyMap, queryNode));

      //std::cout << "First: " << *first << " : " << get(PropertyMap, *first) << " query: " << queryNode << " : " << get(PropertyMap, queryNode) << std::endl;

      // If the distance is not less than infinity, it is useless, so do not continue
      if(!CompareFunction(d, std::numeric_limits<DistanceValueType>::infinity()))
      {
        continue;
      }

      if(CompareFunction(d, DistanceThreshold))
      {
        // std::cout << d << " was less than " << DistanceThreshold << std::endl;
        output.push_back(*iter);
      }
      else
      {
        // std::cout << d << " was NOT less than " << DistanceThreshold << std::endl;
      }
    }
    
    std::cout << output.size() << " items passed the threshold test." << std::endl;
    
    // If no items passed the threshold, we must return all of the items for further inspection in a later step.
    if(output.size() <= 0)
      {
      for(ForwardIteratorType iter = first; iter != last; ++iter)
        {
        output.push_back(*iter);
        }
      }
      
    OutputPassingRegion(output);
  }

  template <typename OutputContainerType>
  void OutputPassingRegion(const OutputContainerType& output)
  {
//     itk::Image<unsigned char, 2>::Pointer image = itk::Image<unsigned char, 2>::New();
//     itk::Index<2> index = {{0,0}};
//     itk::Size<2> size = // How to get the size at this point?
//     
//     itk::ImageRegion<2> region;
//     image->SetRegions(region);
  }
};

#endif
