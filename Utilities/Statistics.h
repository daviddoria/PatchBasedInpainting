#ifndef Statistics_H
#define Statistics_H

// ITK
#include "itkVariableLengthVector.h"

namespace Statistics
{
/** Set up traits determine larger types for computation when necessary. */
template <class T>
struct TypeTraits
{
  typedef T InternalType;
};

template <>
struct TypeTraits<unsigned char>
{
  typedef float InternalType;
};

template <>
struct TypeTraits<itk::VariableLengthVector<unsigned char> >
{
  typedef itk::VariableLengthVector<float> InternalType;
};

/** Average the values in a vector. */
template<typename TVector>
typename TypeTraits<typename TVector::value_type>::InternalType Average(const TVector& v);

/** Average the values in a vector without summing them all first. */
template<typename TVector>
typename TypeTraits<typename TVector::value_type>::InternalType RunningAverage(const TVector& v);

/** Compute the variance of the values in a vector. */
template<typename TVector>
typename TypeTraits<typename TVector::value_type>::InternalType Variance(const TVector& v);

}

#include "Statistics.hpp"

#endif
