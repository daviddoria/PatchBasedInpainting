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

#ifndef PixelFeatureVectorDescriptorVisitor_HPP
#define PixelFeatureVectorDescriptorVisitor_HPP

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

#include "Visitors/InpaintingVisitorParent.h"

#include "Concepts/DescriptorConcept.hpp"
#include "Visitors/DescriptorVisitors/DescriptorVisitorParent.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Submodules
#include <ITKHelpers/ITKHelpers.h>

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It creates
 * and differences FeatureVectorPixelDescriptor objects at each pixel.
 */
template <typename TGraph, typename TImage, typename TDescriptorMap>
struct PixelFeatureVectorDescriptorVisitor : public DescriptorVisitorParent<TGraph>
{
  BOOST_CONCEPT_ASSERT((DescriptorConcept<FeatureVectorPixelDescriptor, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TDescriptorMap& DescriptorMap;

  unsigned int HalfWidth;

  PixelFeatureVectorDescriptorVisitor(TImage* const image, Mask* const mask,
                              TDescriptorMap& descriptorMap, const unsigned int halfWidth) :
  Image(image), MaskImage(mask), DescriptorMap(descriptorMap), HalfWidth(halfWidth)
  {
  }

  void InitializeVertex(VertexDescriptorType v, TGraph& g) const override
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index;
    index[0] = v[0];
    index[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->HalfWidth);

    unsigned int numberOfValues = Image->GetNumberOfComponentsPerPixel() * region.GetNumberOfPixels();
    if(this->MaskImage->IsValid(region))
    {
      std::vector<float> pixels(numberOfValues, 0);
      itk::ImageRegionIterator<TImage> imageIterator(Image, region);

      unsigned int pixelCounter = 0;
      while(!imageIterator.IsAtEnd())
      {
        TImage::PixelType p = imageIterator.Get();
        for(unsigned int component = 0; component < p.Size(); ++component)
        {
          pixels[Image->GetNumberOfComponentsPerPixel() * pixelCounter + component] = p[component];
        }
        pixelCounter++;
        ++imageIterator;
      }
      FeatureVectorPixelDescriptor descriptor(this->Image, this->Mask, region);
      descriptor.SetVertex(v);
      put(this->DescriptorMap, v, descriptor);
    }
    else
    {
      FeatureVectorPixelDescriptor descriptor(numberOfValues);
      descriptor.SetVertex(v);
      put(this->DescriptorMap, v, descriptor);
    }

  }

  void DiscoverVertex(VertexDescriptorType v, TGraph& g) const override
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

    // std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    DescriptorType& descriptor = get(this->DescriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    descriptor.SetValidOffsets(validOffsets);
  }

}; // ImagePatchDescriptorVisitor

#endif
