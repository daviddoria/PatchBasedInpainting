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

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->HalfWidth);

    DescriptorType descriptor(this->Image, this->MaskImage, region);
    descriptor.SetVertex(v);
    put(this->DescriptorMap, v, descriptor);

    // This is done in the descriptor constructor
//     if(mask->IsValid(region))
//       {
//       descriptor.SetStatus(DescriptorType::SOURCE_NODE);
//       }
  }

  void DiscoverVertex(VertexDescriptorType v) const
  {
    itk::Index<2> index = {{v[0], v[1]}};
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->HalfWidth);

    // Allow target patches to be not entirely inside the image.
    // This occurs when the hole boundary is near the image boundary.
//    region.Crop(this->MaskImage->GetLargestPossibleRegion()); // Don't do this! We must maintain
    // the actual location of the patch (so that the vertex 'v' is at the center of the 'region'.
    // This cropping must be done everywhere else, so that the source patches can be cropped
    // the same as the target patch (this one).


    itk::ImageRegion<2> croppedRegion = region;
    croppedRegion.Crop(this->MaskImage->GetLargestPossibleRegion());

    itk::Offset<2> croppedOffset = croppedRegion.GetIndex() - region.GetIndex();

    // Create the list of valid pixels
    std::vector<itk::Index<2> > validPixels = this->MaskImage->GetValidPixelsInRegion(croppedRegion);

    // Create the list of offsets relative to the original region (before cropping)
    std::vector<itk::Offset<2> > validOffsets;
    for(size_t i = 0; i < validPixels.size(); ++i)
    {
      // Compute the offset relative to the cropped region
      itk::Offset<2> offset = validPixels[i] - region.GetIndex();

      // Store the offset relative to the original region
      validOffsets.push_back(offset - croppedOffset);
    }

    // std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(this->DescriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    descriptor.SetValidOffsets(validOffsets);
  }

}; // ImagePatchDescriptorVisitor

#endif
