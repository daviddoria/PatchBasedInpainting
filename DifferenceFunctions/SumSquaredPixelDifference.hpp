/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef SumSquaredPixelDifference_hpp
#define SumSquaredPixelDifference_hpp

// STL
#include <iostream>
#include <stdexcept>

// Custom
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKContainerInterface.h"

/** These classes take two pixels and compute their SSD. There is a generic version, and specializations optimized for specific pixel types.*/

/**
  * This is the generic version of the SSD function that can take any type of pixel with a Helpers::length and Helpers::index implementation.
  */
template <typename PixelType>
class SumSquaredPixelDifference
{
public:
  inline float operator()(const PixelType& a, const PixelType& b) const
  {
    std::cout << "SumSquaredPixelDifference[Generic]::operator()" << std::endl;
    assert(Helpers::length(a) == Helpers::length(b));

    float pixelDifference = 0.0f;
    for(unsigned int component = 0; component < Helpers::length(a); ++component)
    {
      float componentDifference = (Helpers::index(a,component) - Helpers::index(b,component)) *
          (Helpers::index(a,component) - Helpers::index(b,component));
      pixelDifference += componentDifference;
    }
    return pixelDifference;
  }
};

/**
  * This specialization handles pixels of type CovariantVector<unsigned char,N>. This is different from the specialization
  * for CovariantVector<T, N> because it has to first convert the values to float.
  */
template <>
template <unsigned int N>
class SumSquaredPixelDifference <itk::CovariantVector<unsigned char, N> >
{
public:
  typedef itk::CovariantVector<unsigned char, N> PixelType;

//  float operator()(const PixelType& a, const PixelType& b)// const // This cannot be 'const' because of the method we are using to store the float pixels (unless we use mutable (see member variable declarations)
  inline float operator()(const PixelType& a, const PixelType& b) const
  {
    std::cout << "SumSquaredPixelDifference <itk::CovariantVector<unsigned char, N> >::operator()" << std::endl;
    this->A = a;
    this->B = b;
    return (this->A - this->B).GetSquaredNorm();
  }

private:
  // These variables are created as member variables so that they do not have to be allocated at each call to operator()
  typedef itk::CovariantVector<float, N> FloatPixelType;
  mutable FloatPixelType A;
  mutable FloatPixelType B;
};

/**
  * This specialization handles pixels of type CovariantVector<T,N>.
  */
template<typename T, unsigned int N, int i >
class SquaredChannelDifference
{
  public:
    static inline float EXEC(const itk::CovariantVector<T, N>& a, const itk::CovariantVector<T, N>& b)
    {
      return (a[i] - b[i]) * (a[i] - b[i]) + SquaredChannelDifference<T, N, i-1 >::EXEC(a,b);
    }
};

template<typename T, unsigned int N>
class SquaredChannelDifference<T, N, 0>
{
  public:
    static inline float EXEC(const itk::CovariantVector<T, N>& a, const itk::CovariantVector<T, N>& b)
    {
      return (a[0] - b[0]) * (a[0] - b[0]);
    }
};

// Unrolled version
template <>
template <typename T, unsigned int N>
class SumSquaredPixelDifference <itk::CovariantVector<T, N> >
{
public:
  typedef itk::CovariantVector<T, N> PixelType;

  inline float operator()(const PixelType& a, const PixelType& b) const
  {
    return SquaredChannelDifference<T, N, N-1>::EXEC(a,b); // Call with i=N-1, because for example we want to start on element 2 (zero indexed) if the vector is dimension 3
  }
};

// Non-unrolled version
//template <>
//template <typename T, unsigned int N>
//class SumSquaredPixelDifference <itk::CovariantVector<T, N> >
//{
//public:
//  typedef itk::CovariantVector<T, N> PixelType;

//  // Using ITK function. This function uses casts to double internally, so it is not ideal for integer types
//  // with values that you know will not cause overflow (unnecessarily slow)
////  float operator()(const PixelType& a, const PixelType& b) const
////  {
//////    std::cout << "SumSquaredPixelDifference <itk::CovariantVector<T, N> >::operator()" << std::endl;
////    return (a-b).GetSquaredNorm();
////  }

//  // Manual
//  float operator()(const PixelType& a, const PixelType& b) const
//  {
////    std::cout << "SumSquaredPixelDifference <itk::CovariantVector<T, N> >::operator()" << std::endl;
//    T sum = 0;

//    for( unsigned int i = 0; i < N; i++)
//    {
//      sum += (a[i] - b[i]) * (a[i] - b[i]);
//    }
//    return sum;
//  }
//};

#endif
