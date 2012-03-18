#ifndef InpaintPatchVisitor_HPP
#define InpaintPatchVisitor_HPP

#include "Visitors/SimpleVisitors/VisitorSuperclass.hpp"

// Helpers
#include "Helpers/ITKHelpers.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/BoostHelpers.h"

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
  };

  void FinishVertex(TVertexDescriptor target, TVertexDescriptor sourceNode) {}
  
  void InpaintingComplete() const
  {
    //OutputHelpers::WriteImage(Image, "InpaintPatchVisitor::output.mha");
  }

}; // InpaintingVisitor

#endif
