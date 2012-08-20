#ifndef FirstValidDescriptor_HPP
#define FirstValidDescriptor_HPP

#include "PixelDescriptors/PixelDescriptor.h"

#include <boost/property_map/property_map.hpp>

template <typename DescriptorMapType>
struct FirstValidDescriptor
{
  DescriptorMapType DescriptorMap;

  FirstValidDescriptor(DescriptorMapType descriptorMap) : DescriptorMap(descriptorMap)
  {
  }

  template <typename TForwardIterator>
  typename TForwardIterator::value_type operator()(TForwardIterator first, TForwardIterator last,
                                                   typename TForwardIterator::value_type query)
  {
    for(TForwardIterator iter = first; iter != last; ++iter)
    {
      if(get(DescriptorMap, *iter).GetStatus() == PixelDescriptor::SOURCE_NODE)
      {
        return *iter;
      }
    };
    throw std::runtime_error("FirstValidDescriptor: There were no valid descriptors!");
  }
};

#endif
