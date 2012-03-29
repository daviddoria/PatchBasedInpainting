#ifndef ImagePatchVectorizedIndicesVisitor_HPP
#define ImagePatchVectorizedIndicesVisitor_HPP

#include "PixelDescriptors/ImagePatchVectorizedIndices.h"

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
struct ImagePatchVectorizedIndicesVisitor : public DescriptorVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TDescriptorMap& DescriptorMap;

  unsigned int HalfWidth;

  ImagePatchVectorizedIndicesVisitor(TImage* const in_image, Mask* const in_mask,
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

    std::vector<unsigned int> validOffsets;

    itk::ImageRegionConstIteratorWithIndex<Mask> iterator(MaskImage, region);

    unsigned int linearCounter = 0;
    while(!iterator.IsAtEnd())
      {
      if(MaskImage->IsValid(iterator.GetIndex()))
        {
        validOffsets.push_back(linearCounter);
        }
      linearCounter++;
      ++iterator;
      }

    // std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(DescriptorMap, v);
    descriptor.CreateIndexVector();
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    descriptor.SetValidOffsets(validOffsets);
  };

}; // ImagePatchVectorizedVisitor

#endif
