/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef PixelWiseDifferencePatchPairVisitor_H
#define PixelWiseDifferencePatchPairVisitor_H

#include "PatchPairVisitor.h"

#include "Mask.h"

/**
\class PixelWiseDifferencePatchPairVisitor
\brief This class visits PatchPairs.
*/
template <typename TPixelPairVisitor>
class PixelWiseDifferencePatchPairVisitor : public PatchPairVisitor
{
public:
  PixelWiseDifferencePatchPairVisitor(const typename TPixelPairVisitor::ImageType* const image, const Mask* const mask) : Image(image), MaskImage(mask)
  {
  }

  void Visit(const PatchPair& patchPair)
  {
    TPixelPairVisitor pixelPairVisitor;
    patchPair.VisitAllPixels(Image, pixelPairVisitor);
  }

private:
  const typename TPixelPairVisitor::ImageType* Image;
  const Mask* MaskImage;
};

#endif
