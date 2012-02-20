#ifndef BoostHelpers_HPP
#define BoostHelpers_HPP

#include "BoostHelpers.h" // Appease syntax parser

// STL
#include <vector>

// Boost
#include <boost/graph/detail/d_ary_heap.hpp>

// ITK
#include "itkImageRegionConstIterator.h"

// Custom
#include "Helpers/ITKHelpers.h"

namespace BoostHelpers
{
  // Destruct and reconstruct the queue
//   template <typename TQueue>
//   void OutputQueue(TQueue& queue)
//   {
//     std::vector<typename TQueue::value_type> objects;
//     
//     typename TQueue::key_map propertyMap = queue.keys();
// 
//     // Read out the queue, saving the objects in the order they were in the queue
//     while(!queue.empty())
//     {
//       typename TQueue::value_type queuedObject = queue.top();
//       objects.push_back(queuedObject);
//       std::cout << "queuedObject: " << queuedObject << " ";
//       typename boost::property_traits<typename TQueue::key_map>::value_type value = get(propertyMap, value);
//       std::cout << " value: " << value << std::endl;
//       queue.pop();
//     }
//     
//     // Reconstruct the queue
//     for(unsigned int i = 0; i < objects.size(); ++i)
//       {
//       queue.push(objects[i]);
//       }
//   }

  // Copy by value, so we don't care that we pop everything out of the queue
  template <typename TQueue>
  void OutputQueue(TQueue queue)
  {
    typename TQueue::key_map propertyMap = queue.keys();

    // Read out the queue, saving the objects in the order they were in the queue
    while(!queue.empty())
    {
      typename TQueue::value_type queuedObject = queue.top();
      std::cout << "queuedObject: " << queuedObject << " ";
      typename boost::property_traits<typename TQueue::key_map>::value_type value = get(propertyMap, queuedObject);
      std::cout << " value: " << value << std::endl;
      queue.pop();
    }
  }

  template <typename TPropertyMap, typename TImage>
  void WritePropertyMapAsImage(TPropertyMap propertyMap, TImage* const image, const std::string& fileName)
  {
    typename itk::ImageRegionIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

    while(!imageIterator.IsAtEnd())
      {
      typename TPropertyMap::key_type v =
          Helpers::ConvertFrom<typename TPropertyMap::key_type, itk::Index<2> >(imageIterator.GetIndex());
      int value = 0;
      if(get(propertyMap, v))
        {
          value = 255;
        }
      imageIterator.Set(value);
      ++imageIterator;
      }
    typedef typename itk::ImageFileWriter<TImage> WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetInput(image);
    writer->SetFileName(fileName);
    writer->Write();
  }
}

#endif
