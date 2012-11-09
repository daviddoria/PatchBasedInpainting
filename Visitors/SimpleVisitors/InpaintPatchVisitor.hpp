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

#ifndef InpaintPatchVisitor_HPP
#define InpaintPatchVisitor_HPP

#include "Visitors/SimpleVisitors/VisitorSuperclass.hpp"

// Helpers
#include <ITKHelpers/ITKHelpers.h>

template <typename TVertexDescriptor, typename TImage>
struct InpaintPatchVisitor : public VisitorSuperclass<TVertexDescriptor>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int PatchHalfWidth;

  InpaintPatchVisitor(TImage* const image, Mask* const mask,
                      const unsigned int patchHalfWidth) :
  Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth)
  {
  }

  void PaintPatch(TVertexDescriptor target, TVertexDescriptor source) const
  {
    TVertexDescriptor target_patch_corner;
    target_patch_corner[0] = target[0] - PatchHalfWidth;
    target_patch_corner[1] = target[1] - PatchHalfWidth;

    TVertexDescriptor source_patch_corner;
    source_patch_corner[0] = source[0] - PatchHalfWidth;
    source_patch_corner[1] = source[1] - PatchHalfWidth;

    TVertexDescriptor target_node;
    TVertexDescriptor source_node;
    for(std::size_t i = 0; i < PatchHalfWidth * 2 + 1; ++i)
    {
      for(std::size_t j = 0; j < PatchHalfWidth * 2 + 1; ++j)
      {
        target_node[0] = target_patch_corner[0] + i;
        target_node[1] = target_patch_corner[1] + j;

        source_node[0] = source_patch_corner[0] + i;
        source_node[1] = source_patch_corner[1] + j;

        // Only paint the pixel if it is currently a hole
        if( MaskImage->IsHole(ITKHelpers::CreateIndex(target_node)) )
        {
          //std::cout << "Copying pixel " << source_node << " to pixel " << target_node << std::endl;
          itk::Index<2> target_index = ITKHelpers::CreateIndex(target);

          itk::Index<2> source_index = ITKHelpers::CreateIndex(source);

          assert(Image->GetLargestPossibleRegion().IsInside(source_index));
          assert(Image->GetLargestPossibleRegion().IsInside(target_index));

          Image->SetPixel(target_index, Image->GetPixel(source_index));
        }

      }
    }
  }

  void FinishVertex(TVertexDescriptor target, TVertexDescriptor sourceNode) {}
  
  void InpaintingComplete() const
  {
    //OutputHelpers::WriteImage(Image, "InpaintPatchVisitor::output.mha");
  }

}; // InpaintingVisitor

#endif
