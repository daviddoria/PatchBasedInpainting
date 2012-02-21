#ifndef Statistics_H
#define Statistics_H

// Custom
#include "Utilities/TypeTraits.h"

namespace Statistics
{

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
