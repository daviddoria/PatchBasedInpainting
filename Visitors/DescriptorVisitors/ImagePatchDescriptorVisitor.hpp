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

// STL
#include <memory>

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
  std::shared_ptr<TDescriptorMap> DescriptorMap;

  unsigned int HalfWidth;

  ImagePatchDescriptorVisitor(TImage* const in_image, Mask* const in_mask,
                              std::shared_ptr<TDescriptorMap> in_descriptorMap,
                              const unsigned int in_half_width) :
    Image(in_image), MaskImage(in_mask), DescriptorMap(in_descriptorMap),
    HalfWidth(in_half_width)
  {
  }

  void InitializeVertex(VertexDescriptorType v) const
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index = ITKHelpers::CreateIndex(v);

    // It is ok if this region is partially outside the image - we do the cropping in later functions.
    // We cannot crop here because then we lose the relative position of the remaining piece of the target
    // patch relative to full valid patches.
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->HalfWidth);

    DescriptorType descriptor(this->Image, this->MaskImage, region);
    descriptor.SetVertex(v);
    put(*(this->DescriptorMap), v, descriptor);

    // This is done in the descriptor constructor ("DescriptorType descriptor" above)
//     if(mask->IsValid(region))
//       {
//       descriptor.SetStatus(DescriptorType::SOURCE_NODE);
//       }
  }

  void DiscoverVertex(VertexDescriptorType v) const
  {
    itk::Index<2> center = ITKHelpers::CreateIndex(v);
//    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(center, this->HalfWidth);
    itk::ImageRegion<2> region = get(*(this->DescriptorMap), v).GetRegion();

    // Allow target patches to be not entirely inside the image.
    // This occurs when the hole boundary is near the image boundary.

    // This cropping must be done everywhere else, so that the source patches can be cropped
    // the same as the target patch (this one).

    itk::ImageRegion<2> croppedRegion = region;
    croppedRegion.Crop(this->MaskImage->GetLargestPossibleRegion());

    // Create the list of valid pixels
    std::vector<itk::Index<2> > validPixels = this->MaskImage->GetValidPixelsInRegion(croppedRegion);

    // Create the list of offsets relative to the original region (before cropping)
    std::vector<itk::Offset<2> > validOffsets;
    for(size_t i = 0; i < validPixels.size(); ++i)
    {
      // Compute the offset relative to the cropped region
      itk::Offset<2> offsetFromPatchCorner = validPixels[i] - region.GetIndex();

      // Store the offset relative to the original region
      validOffsets.push_back(offsetFromPatchCorner);
    }

    // std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(*(this->DescriptorMap), v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    descriptor.SetValidOffsets(validOffsets);
  }

}; // end class ImagePatchDescriptorVisitor

#endif
