#ifndef PriorityManual_H
#define PriorityManual_H

#include "Priority.h"
#include "PriorityOnionPeel.h"
#include "PriorityRandom.h"

/**
\class PriorityManual
\brief This example of a Priority subclass shows how complicated a Priority class can get.
       It is a "two step" priority computation - where first it uses a new technique (dependent
       on a user supplied image) then falls back onto another Priority subclass.
       This class first returns values that have been specified by a user generated image
       that contains pixels that indicate "definitely fill first". Once there are no more of
       these pixels, the superclass's ComputePriority function is called.
       This is a "template template" class because one of the template parameters (TPriority) depends on another (TImage).
*/
class PriorityManual
{
public:
  typedef itk::Image<unsigned char, 2> ManualPriorityImageType;

  // Reimplemented
  PriorityManual(ManualPriorityImageType* manualPriorityImage, Priority* const priorityFunction) :
  ManualPriorityImage(manualPriorityImage), FallbackPriorityFunction(priorityFunction){}

  float ComputePriority(const itk::Index<2>& queryPixel) const;

  // New functions
  void SetManualPriorityImage(ManualPriorityImageType* const);

  void Update(const itk::ImageRegion<2>& filledRegion);

  void SetFallbackPriorityFunction(Priority* const priorityFunction)
  {
    this->FallbackPriorityFunction = priorityFunction;
  }

protected:
  ManualPriorityImageType* ManualPriorityImage;

  const Priority* FallbackPriorityFunction;
};

#endif
