#ifndef PriorityRandom_HPP
#define PriorityRandom_HPP

#include "Priority/PriorityRandom.h" // Appease syntax parser

#include <cstdlib> // drand48()
#include <ctime> // time()

PriorityRandom::PriorityRandom()
{
  srand48((unsigned)time(0));
}

PriorityRandom::PriorityRandom(const bool random)
{
  if(random)
  {
    srand48((unsigned)time(0));
  }
  else
  {
    srand48(0);
  }
}

template <typename TNode>
float PriorityRandom::ComputePriority(const TNode& queryNode) const
{
  return drand48();
}

#endif
