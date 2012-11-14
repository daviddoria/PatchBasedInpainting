#ifndef Statistics_H
#define Statistics_H

// Custom
#include "Utilities/TypeTraits.h"

namespace Statistics
{

/** Average the values in a vector. */
template<typename TVector>
typename TypeTraits<TVector>::LargerComponentType Average(const TVector& v);

/** Average the values in a vector without summing them all first. */
template<typename TVector>
typename TypeTraits<TVector>::LargerComponentType RunningAverage(const TVector& v);

/** Compute the variance of the values in a vector. */
template<typename TVector>
typename TypeTraits<TVector>::LargerComponentType Variance(const TVector& v);

}

#include "Statistics.hpp"

#endif
