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
  float operator()(const PixelType& a, const PixelType& b) const
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
  float operator()(const PixelType& a, const PixelType& b) const
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
template <>
template <typename T, unsigned int N>
class SumSquaredPixelDifference <itk::CovariantVector<T, N> >
{
public:
  typedef itk::CovariantVector<T, N> PixelType;

  // Using ITK function. This function uses casts to double internally, so it is not ideal for integer types
  // with values that you know will not cause overflow (unnecessarily slow)
//  float operator()(const PixelType& a, const PixelType& b) const
//  {
////    std::cout << "SumSquaredPixelDifference <itk::CovariantVector<T, N> >::operator()" << std::endl;
//    return (a-b).GetSquaredNorm();
//  }

  // Manual
  float operator()(const PixelType& a, const PixelType& b) const
  {
//    std::cout << "SumSquaredPixelDifference <itk::CovariantVector<T, N> >::operator()" << std::endl;
    T sum = 0;

    for( unsigned int i = 0; i < N; i++)
    {
      sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sum;
  }


};

#endif
