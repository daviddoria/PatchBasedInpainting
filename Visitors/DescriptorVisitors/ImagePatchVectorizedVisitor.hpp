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

#ifndef ImagePatchVectorizedVisitor_HPP
#define ImagePatchVectorizedVisitor_HPP

#include "PixelDescriptors/ImagePatchVectorized.h"

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
struct ImagePatchVectorizedVisitor : public DescriptorVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TDescriptorMap& DescriptorMap;

  unsigned int HalfWidth;

  ImagePatchVectorizedVisitor(TImage* const in_image, Mask* const in_mask,
                              TDescriptorMap& in_descriptorMap, const unsigned int in_half_width) :
  Image(in_image), MaskImage(in_mask), DescriptorMap(in_descriptorMap), HalfWidth(in_half_width)
  {
  }

  void InitializeVertex(VertexDescriptorType v) const override
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index = ITKHelpers::CreateIndex(v);

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, HalfWidth);

    DescriptorType descriptor(this->Image, this->MaskImage, region);
    descriptor.SetVertex(v);
    put(this->DescriptorMap, v, descriptor);

    // This is done in the descriptor constructor
//     if(mask->IsValid(region))
//       {
//       descriptor.SetStatus(DescriptorType::SOURCE_NODE);
//       }
  }

  void DiscoverVertex(VertexDescriptorType v) const override
  {
    itk::Index<2> index = {{v[0], v[1]}};
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->HalfWidth);

    // Create the list of valid pixels
    
    std::vector<unsigned int> validOffsets;

    itk::ImageRegionConstIteratorWithIndex<Mask> iterator(this->MaskImage, region);

    unsigned int linearCounter = 0;
    while(!iterator.IsAtEnd())
    {
      if(this->MaskImage->IsValid(iterator.GetIndex()))
      {
        validOffsets.push_back(linearCounter);
      }
      linearCounter++;
      ++iterator;
    }

    // std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(this->DescriptorMap, v);
    descriptor.CreatePixelVector();
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    descriptor.SetValidOffsets(validOffsets);
  }

}; // ImagePatchVectorizedVisitor

#endif
