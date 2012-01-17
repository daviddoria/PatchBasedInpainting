#include "PatchBasedInpainting.h" // Appease syntax parser

template <typename TImage>
PatchBasedInpainting<TImage>::PatchBasedInpainting(const TImage* const image, const Mask* mask) : Inpainting<TImage>(image, mask)
{

}

template <typename TImage>
void PatchBasedInpainting<TImage>::Inpaint()
{

}
