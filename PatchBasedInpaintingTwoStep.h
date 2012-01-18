#ifndef PatchBasedInpaintingTwoStep_h
#define PatchBasedInpaintingTwoStep_h

#include "PatchBasedInpainting.h"

/**
\class PatchBasedInpaintingTwoStep
\brief This class perform image inpainting by copying patches from elsewhere in the image into the hole.
       The DetermineBestSourcePixel function is more complicated than in the parent class.
*/
template <typename TImage>
class PatchBasedInpaintingTwoStep : public PatchBasedInpainting<TImage>
{
public:

  PatchBasedInpaintingTwoStep(const TImage* const image, const Mask* mask);

protected:

  itk::Index<2> DetermineBestSourcePixel(itk::Index<2> targetPixel);
};

#include "PatchBasedInpaintingTwoStep.hxx"

#endif
