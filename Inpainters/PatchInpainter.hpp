/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef PatchInpainter_HPP
#define PatchInpainter_HPP

#include "PatchInpainterParent.h"

template <typename TImage>
class PatchInpainter : public PatchInpainterParent
{
  /** The image to inpaint. */
  TImage* Image;

  /** The mask to use to determine which pixels are holes (which pixels to inpaint). */
  const Mask* MaskImage;

  /** The size of the patches used in the inpainting. */
  std::size_t PatchHalfWidth;

  // Debug only
  /** Count how many times this classes function has been performed. */
  unsigned int Iteration;

  /** A flag indicating if we would like to write debug images. */
  bool DebugImages;

  /** The name of the image as the programmer would refer to it. I.e. "TheMask" or similar. */
  std::string ImageName;

public:

  /** Copy the pixel value at sourceIndex to targetIndex. */
  void PaintVertex(const itk::Index<2>& targetIndex, const itk::Index<2>& sourceIndex)
  {
    this->Image->SetPixel(targetIndex, this->Image->GetPixel(sourceIndex));
  }

  /** Specify a name for the image. */
  void SetImageName(const std::string& imageName)
  {
    this->ImageName = imageName;
  }

  /** Determine if the debug images should be written. */
  void SetDebugImages(const bool debugImages)
  {
    this->DebugImages = debugImages;
  }

  /** Inpaint a patch only in the masked pixels. */
  PatchInpainter(std::size_t patchHalfWidth, TImage* const image, const Mask* const mask) :
    Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth), Iteration(0), DebugImages(false), ImageName("Unnamed")
  {
//    std::cout << "PatchInpainter: size: " << this->Image->GetLargestPossibleRegion().GetSize() << std::endl;
  }

  void PaintPatch(const itk::Index<2>& targetCenter, const itk::Index<2>& sourceCenter)
  {
    assert(this->Image);

//    std::cout << "PatchInpainter: Mask size: " << this->MaskImage->GetLargestPossibleRegion().GetSize() << std::endl;

    // Compute the corners of the patches from their centers and the PatchHalfWidth
    itk::Index<2> targetPatchCorner;
    targetPatchCorner[0] = targetCenter[0] - this->PatchHalfWidth;
    targetPatchCorner[1] = targetCenter[1] - this->PatchHalfWidth;

    itk::Index<2> sourcePatchCorner;
    sourcePatchCorner[0] = sourceCenter[0] - this->PatchHalfWidth;
    sourcePatchCorner[1] = sourceCenter[1] - this->PatchHalfWidth;

    // Iterate over all pixels in the patch
    itk::Index<2> targetNode;
    itk::Index<2> sourceNode;
    for(std::size_t i = 0; i < this->PatchHalfWidth * 2 + 1; ++i)
    {
      for(std::size_t j = 0; j < this->PatchHalfWidth * 2 + 1; ++j)
      {
        targetNode[0] = targetPatchCorner[0] + i;
        targetNode[1] = targetPatchCorner[1] + j;

        sourceNode[0] = sourcePatchCorner[0] + i;
        sourceNode[1] = sourcePatchCorner[1] + j;

        // Only paint the pixel if it is currently a hole
        if( this->MaskImage->IsHole(targetNode) )
        {
          PaintVertex(targetNode, sourceNode);
        }
      }
    }

    if(this->DebugImages)
    {
      std::cout << "PatchInpainter::PaintPatch(): Painted patch " << targetCenter[0] << " " << targetCenter[1]
                << " with " << sourceCenter[0] << " " << sourceCenter[1] << std::endl;
      std::cout << "PatchInpainter::PaintPatch() After filling, there are " << this->MaskImage->CountHolePixels() << " hole pixels remaining." << std::endl;

      try
      {
        ITKHelpers::WriteSequentialImage(this->Image, this->ImageName, this->Iteration, 3, "png");
      }
      catch (...)
      {
        ITKHelpers::WriteSequentialRGBImage(this->Image, this->ImageName, this->Iteration, 3, "png");
      }
    }

    this->Iteration++;
  } // end operator()

}; // end class PatchInpainter

#endif
