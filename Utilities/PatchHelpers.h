#ifndef PatchHelpers_H
#define PatchHelpers_H

// Qt
#include <QImage>

// Submodules
#include <Mask/Mask.h>

namespace PatchHelpers
{

////////////// Non template functions //////////////
bool CheckSurroundingRegionsOfAllHolePixels(const Mask* const mask, const unsigned int patchRadius);

////////////// Template functions //////////////
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

template <class TPriorityQueue>
void WritePriorityQueue(TPriorityQueue q, const std::string& fileName);

/** Write an image of the priorities at the pixels that are valid in the propertyMap */
template <typename TNodeQueue, typename TPropertyMap>
void WriteValidQueueNodesPrioritiesImage(TNodeQueue nodeQueue, const TPropertyMap propertyMap,
                                         const itk::ImageRegion<2>& fullRegion, const std::string& fileName);

/** Write an image of the priorities at the pixels that are valid in the propertyMap */
template <typename TNodeQueue, typename TPropertyMap>
void WriteValidQueueNodesLocationsImage(TNodeQueue nodeQueue, const TPropertyMap propertyMap,
                                       const itk::ImageRegion<2>& fullRegion, const std::string& fileName);

}

#include "PatchHelpers.hpp"

#endif
