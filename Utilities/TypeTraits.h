#ifndef TypeTraits_H
#define TypeTraits_H

// ITK
#include "itkVariableLengthVector.h"

// STL
#include <vector>

/** Set up traits determine larger types for computation when necessary. */
template <class T>
struct TypeTraits
{
  typedef T InternalType;
  typedef T ComponentType;
};

template <>
struct TypeTraits<unsigned char>
{
  typedef float InternalType;
  typedef unsigned char ComponentType;
};

template <typename T>
struct TypeTraits<itk::VariableLengthVector<T> >
{
  typedef itk::VariableLengthVector<float> InternalType;
  typedef typename T::ValueType ComponentType;
};

template <>
struct TypeTraits<itk::VariableLengthVector<unsigned char> >
{
  typedef itk::VariableLengthVector<float> InternalType;
  typedef itk::VariableLengthVector<unsigned char>::ValueType ComponentType;

};

#endif
