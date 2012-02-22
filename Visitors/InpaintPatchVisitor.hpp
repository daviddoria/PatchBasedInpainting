#ifndef InpaintingVisitor_HPP
#define InpaintingVisitor_HPP

// Helpers
#include "Helpers/ITKHelpers.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/BoostHelpers.h"

template <typename TImage>
struct InpaintPatchVisitor
{
  BOOST_CONCEPT_ASSERT((DescriptorVisitorConcept<TDescriptorVisitor, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;

  InpaintingVisitor(TImage* const image, Mask* const mask,
                    const unsigned int halfWidth) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth)
  {
  }

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const
  {
    itk::Index<2> target_index = ITKHelpers::CreateIndex(target);

    itk::Index<2> source_index = ITKHelpers::CreateIndex(source);

    assert(Image->GetLargestPossibleRegion().IsInside(source_index));
    assert(Image->GetLargestPossibleRegion().IsInside(target_index));

    Image->SetPixel(target_index, Image->GetPixel(source_index));
  };

  void InpaintingComplete() const
  {
    OutputHelpers::WriteImage(Image, "output.mha");
  }

}; // InpaintingVisitor

#endif
