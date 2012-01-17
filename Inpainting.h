#ifndef Inpainting_h
#define Inpainting_h

#include "Mask.h"

/**
\class Inpainting
\brief This is an abstract class to perform image inpainting.
*/
template <typename TImage>
class Inpainting
{
public:
  Inpainting(const TImage* const image, const Mask* mask) : Image(image), MaskImage(mask){}

  virtual void Inpaint() = 0;

private:
  const TImage* Image;
  const Mask* MaskImage;
  TImage* Result;
};

#endif
