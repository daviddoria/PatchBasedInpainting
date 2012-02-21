#ifndef Statistics_HPP
#define Statistics_HPP

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"

// STL
#include <iostream>

// ITK
#include "itkNumericTraits.h"

namespace Statistics
{

template<typename TVector>
typename TypeTraits<TVector>::LargerComponentType RunningAverage(const TVector& v)
{
  // If T is not a scalar (i.e. convertible to float (i.e. an itk::VariableLengthVector) )
  // We cannot simply sum all of the values and divide by the number of values, because the type may overflow.
  // E.g. if we sum more than 2 or 3 unsigned chars, we will overflow unsigned char.
  // To remedy this, the average is computed during each step so overflow is always prevented.
  // From: http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average

  typedef typename TypeTraits<TVector>::LargerComponentType AverageType;
  //typedef typename TypeTraits<TVector>::ComponentType ItemType;
  //std::cout << "Helpers::RunningAverage" << std::endl;
  AverageType vectorRunningAverage = v[0]; // We do this because if the length is not known until runtime (std::vector, itk::VariableLengthVector, etc), we want the output to be the right length.
  ITKHelpers::SetObjectToZero(vectorRunningAverage);

  using Helpers::length;
  using ITKHelpers::length;
  for(unsigned int i = 0; i < length(v); ++i)
    {
    //ItemType object = v[i];
    vectorRunningAverage = (static_cast<AverageType>(v[i]) + static_cast<float>(i)*vectorRunningAverage)/static_cast<float>(i+1);

//     for(unsigned int component = 0; component < length(object); ++component);
//       {
//       // std::cout << "Average: Adding value " << v[i] << std::endl;
//       vectorRunningAverage = (static_cast<AverageType>(v[i]) + static_cast<float>(i)*vectorRunningAverage)/static_cast<float>(i+1);
//       //std::cout << "RunningAverage: current average: " << vectorRunningAverage << std::endl;
//       }
    }
  //std::cout << "RunningAverage: final average" << vectorRunningAverage << std::endl;

  return vectorRunningAverage;
}

template<typename TVector>
typename TypeTraits<TVector>::LargerComponentType Average(const TVector& v)
{
  //std::cout << "Helpers::Average" << std::endl;
  typedef typename TypeTraits<TVector>::LargerComponentType AverageType;
  AverageType vectorSum = v[0]; // We do this because if the length is not known until runtime (std::vector, itk::VariableLengthVector, etc), we want the output to be the right length.
  ITKHelpers::SetObjectToZero(vectorSum);

  using Helpers::length;
  using ITKHelpers::length;
  for(unsigned int i = 0; i < length(v); ++i)
    {
    // std::cout << "Average: Adding value " << v[i] << std::endl;

    typedef typename TypeTraits<TVector>::ComponentType ObjectType;
    ObjectType object = v[i];
    vectorSum += object;
  
    //vectorSum += v[i];

  
    //vectorSum = vectorSum + v[i];
    // std::cout << "Average: Current vectorSum " << vectorSum << std::endl;
    }
  // std::cout << "Average: sum " << vectorSum << std::endl;
  //typename T::value_type vectorAverage = vectorSum / static_cast<float>(v.size());
  AverageType vectorAverage = vectorSum / static_cast<float>(v.size());
  
  // std::cout << "Average: average " << vectorAverage << std::endl;

  return vectorAverage;
}

template<typename TVector>
typename TypeTraits<TVector>::LargerComponentType Variance(const TVector& v)
{
  typedef typename TypeTraits<TVector>::LargerComponentType VarianceType;
  
  VarianceType average = Average(v);
  // std::cout << "Variance: average = " << average << std::endl;
  //VarianceType variance = itk::NumericTraits<VarianceType>::Zero; // I don't understand why this doesn't work
  VarianceType variance = v[0]; // We do this because if the length is not known until runtime (std::vector, itk::VariableLengthVector, etc), we want the output to be the right length.
  ITKHelpers::SetObjectToZero(variance);
  // Variance = 1/NumPixels * sum_i (x_i - u)^2

  // std::cout << "Variance: elements have " << itk::NumericTraits<typename TVector::value_type>::GetLength() << " components." << std::endl;

  using Helpers::index;
  using ITKHelpers::index;
  using Helpers::length;
  using ITKHelpers::length;
  for(unsigned int component = 0; component < length(variance); ++component)
  {
    float channelVarianceSummation = 0.0f;
    for(unsigned int i = 0; i < v.size(); ++i)
    {
      channelVarianceSummation += pow(index(v[i], component) -
                                      index(average, component), 2);
    }
    float channelVariance = channelVarianceSummation / static_cast<float>(length(v) - 1); // This (N-1) term in the denominator is for the "unbiased" sample variance. This is what is used by Matlab, Wolfram alpha, etc.
    index(variance, component) = channelVariance;
  }
  return variance;
}

}

#endif
