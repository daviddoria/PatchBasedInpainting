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

  void InitializeVertex(VertexDescriptorType v) const override
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index = ITKHelpers::CreateIndex(v);

    // It is ok if this region is partially outside the image, this is taken care of in the DescriptorType constructor.
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->HalfWidth);

    DescriptorType descriptor(this->Image, this->MaskImage, region);
    descriptor.SetVertex(v);
    descriptor.SetOriginalRegion(region);
    put(*(this->DescriptorMap), v, descriptor);

    // This is done in the descriptor constructor ("DescriptorType descriptor" above)
//     if(mask->IsValid(region))
//       {
//       descriptor.SetStatus(DescriptorType::SOURCE_NODE);
//       }
  }

  void DiscoverVertex(VertexDescriptorType v) const override
  {
    itk::ImageRegion<2> region = get(*(this->DescriptorMap), v).GetRegion();

    itk::ImageRegion<2> croppedRegion = region;
    croppedRegion.Crop(this->MaskImage->GetLargestPossibleRegion());

    // Create the list of valid pixels
    std::vector<itk::Index<2> > validPixels = this->MaskImage->GetValidPixelsInRegion(region);

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
    descriptor.SetRegion(croppedRegion);
    descriptor.SetValidOffsets(validOffsets);
  }

}; // end class ImagePatchDescriptorVisitor

#endif
