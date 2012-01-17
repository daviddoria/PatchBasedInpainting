#ifndef PriorityRandom_H
#define PriorityRandom_H

#include "Priority.h"

/**
\class PriorityRandom
\brief This class returns a random value as the priority of each boundary pixel.
       This example Priority subclass is the easiest - it doesn't rely on any inputs
       and doesn't have an internal state at all.
*/
class PriorityRandom : public Priority
{
public:

  /** Return a random value.*/
  float ComputePriority(const itk::Index<2>& queryPixel) const
  {
    return drand48();
  }

  /** There is no reason to update anything.*/
  void Update(const itk::ImageRegion<2>& filledRegion)
  {
    // There is no reason to update anything.
  }
};

#endif
