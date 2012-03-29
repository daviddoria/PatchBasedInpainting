#ifndef PatchHelpers_H
#define PatchHelpers_H

namespace PatchHelpers
{
template <class TImage>
void CopyRegion(const TImage* const sourceImage, TImage* const targetImage, const itk::Index<2>& sourcePosition,
                const itk::Index<2>& targetPosition, const unsigned int radius);

template <class TImage>
void CopyPatchIntoImage(const TImage* const patch, TImage* const image, const Mask* const mask,
                        const itk::Index<2>& position);


template <class TImage>
void CopyPatchIntoImage(const TImage* patch, TImage* const image, const itk::Index<2>& centerPixel);

}

#include "PatchHelpers.hpp"

#endif
