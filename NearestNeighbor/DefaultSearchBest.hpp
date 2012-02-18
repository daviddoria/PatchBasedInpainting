#ifndef DefaultSearchBest_HPP
#define DefaultSearchBest_HPP

// STL
#include <limits>

struct DefaultSearchBest
{
  template <typename ForwardIteratorType>
  ForwardIteratorType operator()(ForwardIteratorType first, ForwardIteratorType last,
                                 typename ForwardIteratorType::value_type query)
  {
    return *first;
  }
};

#endif
