#ifndef Statistics_HPP
#define Statistics_HPP

// STL
#include <iostream>

// ITK
#include "itkNumericTraits.h"

namespace Statistics
{

template<typename TVector>
typename TypeTraits<typename TVector::value_type>::InternalType RunningAverage(const TVector& v)
{
  // If T is not a scalar (i.e. convertible to float (i.e. an itk::VariableLengthVector) )
  // We cannot simply sum all of the values and divide by the number of values, because the type may overflow.
  // E.g. if we sum more than 2 or 3 unsigned chars, we will overflow unsigned char.
  // To remedy this, the average is computed during each step so overflow is always prevented.
  // From: http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average

  typedef typename TypeTraits<typename TVector::value_type>::InternalType InternalType;
  //std::cout << "Helpers::RunningAverage" << std::endl;
  InternalType vectorRunningAverage = itk::NumericTraits<typename TVector::value_type>::Zero;

  for(unsigned int i = 0; i < v.size(); ++i)
    {
    // std::cout << "Average: Adding value " << v[i] << std::endl;
    vectorRunningAverage = (static_cast<InternalType>(v[i]) + static_cast<float>(i)*vectorRunningAverage)/static_cast<float>(i+1);
    //std::cout << "RunningAverage: current average: " << vectorRunningAverage << std::endl;
    }
  //std::cout << "RunningAverage: final average" << vectorRunningAverage << std::endl;

  return vectorRunningAverage;
}

template<typename TVector>
//typename T::value_type Average(const T& v)
typename TypeTraits<typename TVector::value_type>::InternalType Average(const TVector& v)
{
  //std::cout << "Helpers::Average" << std::endl;
  typename TypeTraits<typename TVector::value_type>::InternalType vectorSum = itk::NumericTraits<typename TVector::value_type>::Zero;

  for(unsigned int i = 0; i < v.size(); ++i)
    {
    // std::cout << "Average: Adding value " << v[i] << std::endl;
    vectorSum += v[i];
    // std::cout << "Average: Current vectorSum " << vectorSum << std::endl;
    }
  // std::cout << "Average: sum " << vectorSum << std::endl;
  //typename T::value_type vectorAverage = vectorSum / static_cast<float>(v.size());
  typename TypeTraits<typename TVector::value_type>::InternalType vectorAverage = vectorSum / static_cast<float>(v.size());
  
  // std::cout << "Average: average " << vectorAverage << std::endl;

  return vectorAverage;
}


}

#endif
