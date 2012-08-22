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

#ifndef MaskImagePatchInpainter_HPP
#define MaskImagePatchInpainter_HPP

#include "ITKHelpers/ITKHelpers.h"

/**
 * This class template is a patch inpainter (i.e. paints the holes in one patch with the values of
 * some given source patch). This class requires that the graph be a 2D grid-graph (like BGL's grid_graph).
 * This class uses the color-map that is given to it to check if the vertices in a patch are holes that need to be filled
 * or not (i.e. it does a masking of the patch around a given target center vertex).
 *
 * \tparam ColorMap A property-map to obtain the vertex color for a given vertex descriptor.
 */
struct MaskImagePatchInpainter
{
  std::size_t patch_half_width;
  const Mask* MaskImage;

  MaskImagePatchInpainter(std::size_t aPatchHalfWidth, const Mask* const mask) :
  patch_half_width(aPatchHalfWidth), MaskImage(mask)
  {
    std::cout << "MaskImagePatchInpainter: Mask size: " << this->MaskImage->GetLargestPossibleRegion().GetSize() << std::endl;
  }

  template <typename Vertex, typename InpaintingVisitor>
  void operator()(Vertex target, Vertex source, InpaintingVisitor vis)
  {
    assert(this->MaskImage);

//    std::cout << "Painting " << target[0] << " " << target[1] << " with " << source[0] << " " << source[1] << std::endl;
//    std::cout << "MaskImagePatchInpainter: Mask size: " << this->MaskImage->GetLargestPossibleRegion().GetSize() << std::endl;

    Vertex target_patch_corner;
    target_patch_corner[0] = target[0] - patch_half_width;
    target_patch_corner[1] = target[1] - patch_half_width;

    Vertex source_patch_corner;
    source_patch_corner[0] = source[0] - patch_half_width;
    source_patch_corner[1] = source[1] - patch_half_width;

    Vertex target_node;
    Vertex source_node;
    for(std::size_t i = 0; i < patch_half_width * 2 + 1; ++i)
    {
      for(std::size_t j = 0; j < patch_half_width * 2 + 1; ++j)
      {
        target_node[0] = target_patch_corner[0] + i;
        target_node[1] = target_patch_corner[1] + j;

        source_node[0] = source_patch_corner[0] + i;
        source_node[1] = source_patch_corner[1] + j;

        // Only paint the pixel if it is currently a hole
        if( this->MaskImage->IsHole(ITKHelpers::CreateIndex(target_node)) )
        {
          //std::cout << "Copying pixel " << source_node << " to pixel " << target_node << std::endl;
          vis.PaintVertex(target_node, source_node); //paint the vertex.
        }

      }
    }
  }

};

#endif
