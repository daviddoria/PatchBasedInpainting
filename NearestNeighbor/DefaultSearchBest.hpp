#ifndef DefaultSearchBest_HPP
#define DefaultSearchBest_HPP

/**
 * This functor has the same signature as other single-best-neightor search functors,
 * but it does not actually do anything - it simply returns the first item in the container.
 * This is often used when the container is already known to be sorted by the same criterion
 * that we would have searched for.
 */
struct DefaultSearchBest
{
  template <typename ForwardIteratorType>
  typename ForwardIteratorType::value_type operator()(ForwardIteratorType first, ForwardIteratorType last,
                                 typename ForwardIteratorType::value_type query)
  {
    return *first;
  }
};

#endif
