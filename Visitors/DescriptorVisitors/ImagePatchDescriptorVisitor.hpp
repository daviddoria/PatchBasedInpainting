#ifndef ImagePatchDescriptorVisitor_HPP
#define ImagePatchDescriptorVisitor_HPP

#include "Priority/Priority.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

#include "Visitors/InpaintingVisitorParent.h"

#include "Concepts/DescriptorConcept.hpp"
#include "Visitors/DescriptorVisitors/DescriptorVisitorParent.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Helpers
#include "Helpers/ITKHelpers.h"

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

  TImage* image;
  Mask* mask;
  TDescriptorMap& descriptorMap;

  unsigned int half_width;

  ImagePatchDescriptorVisitor(TImage* const in_image, Mask* const in_mask,
                              TDescriptorMap& in_descriptorMap, const unsigned int in_half_width) :
  image(in_image), mask(in_mask), descriptorMap(in_descriptorMap), half_width(in_half_width)
  {
  }

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index;
    index[0] = v[0];
    index[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, half_width);

    DescriptorType descriptor(this->image, this->mask, region);
    descriptor.SetVertex(v);
    put(descriptorMap, v, descriptor);

  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const
  {
    itk::Index<2> index = {{v[0], v[1]}};
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, half_width);

    // Create the list of valid pixels
    std::vector<itk::Index<2> > validPixels = mask->GetValidPixelsInRegion(region);
    std::vector<itk::Offset<2> > validOffsets;
    for(size_t i = 0; i < validPixels.size(); ++i)
      {
      itk::Offset<2> offset = validPixels[i] - region.GetIndex();
      validOffsets.push_back(offset);
      }

    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(descriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    descriptor.SetValidOffsets(validOffsets);
  };

}; // ImagePatchDescriptorVisitor

#endif
