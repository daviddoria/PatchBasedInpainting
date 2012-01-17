#ifndef PatchBasedInpainting_h
#define PatchBasedInpainting_h

#include "Inpainting.h"

#include "Priority.h"

/**
\class Inpainting
\brief This class perform image inpainting by copying patches from elsewhere in the image into the hole.
*/
template <typename TImage>
class PatchBasedInpainting : public Inpainting<TImage>
{
public:

  PatchBasedInpainting(const TImage* const image, const Mask* mask);

  void Inpaint();

  void SetPriorityFunction(Priority* priorityFunction)
  {
    this->PriorityFunction = priorityFunction;
  }

private:
  Priority* PriorityFunction;
};

#include "PatchBasedInpainting.hxx"

#endif
