#ifndef PriorityRandom_HPP
#define PriorityRandom_HPP

#include "Priority/PriorityRandom.h" // Appease syntax parser

#include <cstdlib> // drand48()

template <typename TNode>
float PriorityRandom::ComputePriority(const TNode& queryNode) const
{
  return drand48();
}

#endif
