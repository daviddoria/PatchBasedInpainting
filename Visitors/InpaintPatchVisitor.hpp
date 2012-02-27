#ifndef InpaintPatchVisitor_HPP
#define InpaintPatchVisitor_HPP

// Helpers
#include "Helpers/ITKHelpers.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/BoostHelpers.h"

template <typename TImage>
struct InpaintPatchVisitor
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;

  InpaintPatchVisitor(TImage* const image, Mask* const mask,
                    const unsigned int halfWidth) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth)
  {
  }

  template <typename TNode>
  void PaintVertex(TNode target, TNode source) const
  {
    itk::Index<2> target_index = ITKHelpers::CreateIndex(target);

    itk::Index<2> source_index = ITKHelpers::CreateIndex(source);

    assert(Image->GetLargestPossibleRegion().IsInside(source_index));
    assert(Image->GetLargestPossibleRegion().IsInside(target_index));

    Image->SetPixel(target_index, Image->GetPixel(source_index));
  };

  void InpaintingComplete() const
  {
    //OutputHelpers::WriteImage(Image, "InpaintPatchVisitor::output.mha");
  }

}; // InpaintingVisitor

#endif
