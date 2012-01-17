#ifndef Priority_H
#define Priority_H

// ITK
#include "itkIndex.h"
#include "itkImageRegion.h"

/**
\class Priority
\brief This is an abstract class that indicates the interface for Priority functions.
*/
class Priority
{
public:

  /** Compute the priority of a specific pixel. */
  virtual float ComputePriority(const itk::Index<2>& queryPixel) const = 0;

  // At the end of an iteration, update anything that needs to be updated.
  virtual void Update(const itk::ImageRegion<2>& filledRegion) = 0;

};

#endif
