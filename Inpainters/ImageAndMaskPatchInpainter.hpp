#ifndef ImageAndMaskPatchInpainter_HPP
#define ImageAndMaskPatchInpainter_HPP

#include "ITKHelpers/ITKHelpers.h"

/**

 */
template <typename TImage>
struct ImageAndMaskPatchInpainter
{
  TImage* Image;
  Mask* MaskImage;
  std::size_t patch_half_width;

  ImageAndMaskPatchInpainter(TImage* const image, Mask* const mask, std::size_t aPatchHalfWidth) :
  Image(image), MaskImage(mask), patch_half_width(aPatchHalfWidth)
  { };

  template <typename Vertex, typename InpaintingVisitor>
  void operator()(Vertex target, Vertex source, InpaintingVisitor vis)
  {
    std::cout << "Painting " << target[0] << " " << target[1] << " with " << source[0] << " " << source[1] << std::endl;

    Vertex target_patch_corner;
    target_patch_corner[0] = target[0] - patch_half_width;
    target_patch_corner[1] = target[1] - patch_half_width;

    Vertex source_patch_corner;
    source_patch_corner[0] = source[0] - patch_half_width;
    source_patch_corner[1] = source[1] - patch_half_width;

    itk::Index<2> target_index;
    itk::Index<2> source_index;
    for(std::size_t i = 0; i < patch_half_width * 2 + 1; ++i)
    {
      for(std::size_t j = 0; j < patch_half_width * 2 + 1; ++j)
      {
        target_index[0] = target_patch_corner[0] + i;
        target_index[1] = target_patch_corner[1] + j;

        source_index[0] = source_patch_corner[0] + i;
        source_index[1] = source_patch_corner[1] + j;

        // Only paint the pixel if it is currently a hole
        if( MaskImage->IsHole(target_index) )
        {
          Image->SetPixel(target_index, Image->GetPixel(source_index));
          MaskImage->SetPixel(target_index, MaskImage->GetPixel(source_index));
        }

      }
    }
  }

};

#endif
