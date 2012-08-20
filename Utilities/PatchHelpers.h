#ifndef PatchHelpers_H
#define PatchHelpers_H

// Qt
#include <QImage>

// Submodules
#include <Mask/Mask.h>

namespace PatchHelpers
{

template <typename TImage>
QImage GetQImageCombinedPatch(const TImage* const image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, const Mask* const mask);

template <class TImage>
void CopyRegion(const TImage* const sourceImage, TImage* const targetImage, const itk::Index<2>& sourcePosition,
                const itk::Index<2>& targetPosition, const unsigned int radius);

template <class TImage>
void CopyPatchIntoImage(const TImage* const patch, TImage* const image, const Mask* const mask,
                        const itk::Index<2>& position);


template <class TImage>
void CopyPatchIntoImage(const TImage* patch, TImage* const image, const itk::Index<2>& centerPixel);

bool CheckSurroundingRegionsOfAllHolePixels(const Mask* const mask, const unsigned int patchRadius);

}

#include "PatchHelpers.hpp"

#endif
