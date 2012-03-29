#ifndef ImagePatchDescriptorVisitor_HPP
#define ImagePatchDescriptorVisitor_HPP

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

#include "Visitors/InpaintingVisitorParent.h"

#include "Concepts/DescriptorConcept.hpp"
#include "Visitors/DescriptorVisitors/DescriptorVisitorParent.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Helpers
#include "ITKHelpers/ITKHelpers.h"

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It creates
 * and differences ImagePatchPixelDescriptor objects at each pixel.
 */
template <typename TGraph, typename TImage, typename TDescriptorMap>
struct ImagePatchDescriptorVisitor : public DescriptorVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TDescriptorMap& DescriptorMap;

  unsigned int HalfWidth;

  ImagePatchDescriptorVisitor(TImage* const in_image, Mask* const in_mask,
                              TDescriptorMap& in_descriptorMap, const unsigned int in_half_width) :
  Image(in_image), MaskImage(in_mask), DescriptorMap(in_descriptorMap), HalfWidth(in_half_width)
  {
  }

  void InitializeVertex(VertexDescriptorType v) const
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index = ITKHelpers::CreateIndex(v);

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, HalfWidth);

    DescriptorType descriptor(this->Image, this->MaskImage, region);
    descriptor.SetVertex(v);
    put(DescriptorMap, v, descriptor);

    // This is done in the descriptor constructor
//     if(mask->IsValid(region))
//       {
//       descriptor.SetStatus(DescriptorType::SOURCE_NODE);
//       }
  };

  void DiscoverVertex(VertexDescriptorType v) const
  {
    itk::Index<2> index = {{v[0], v[1]}};
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, HalfWidth);

    // Create the list of valid pixels
    std::vector<itk::Index<2> > validPixels = MaskImage->GetValidPixelsInRegion(region);
    std::vector<itk::Offset<2> > validOffsets;
    for(size_t i = 0; i < validPixels.size(); ++i)
      {
      itk::Offset<2> offset = validPixels[i] - region.GetIndex();
      validOffsets.push_back(offset);
      }

    // std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(DescriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    descriptor.SetValidOffsets(validOffsets);
  };

}; // ImagePatchDescriptorVisitor

#endif
