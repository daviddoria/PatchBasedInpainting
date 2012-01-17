#ifndef PatchBasedInpainting_h
#define PatchBasedInpainting_h

#include "Inpainting.h"

#include "Priority.h"

/**
\class PatchBasedInpainting
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

  void Iterate();
  bool IsDone();

  itk::Index<2> DetermineTargetPixel();
  itk::Index<2> DetermineBestSourcePixel(itk::Index<2> targetPixel);
};

#include "PatchBasedInpainting.hxx"

#endif
