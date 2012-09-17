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

#ifndef ImagePatchCreatorVisitor_HPP
#define ImagePatchCreatorVisitor_HPP

// Submodules
#include "ITKHelpers/ITKHelpers.h"

// ITK
#include "itkImage.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

/**
 * This is a simplified visitor type that models with the InpaintingVisitorConcept and does 
 * nothing but create image patches to initialize nodes.
 */
template <typename TImage, typename TDescriptorMap>
struct ImagePatchCreatorVisitor
{
  TImage* image;
  Mask* mask;
  TDescriptorMap& descriptorMap;
  unsigned int half_width;

  ImagePatchCreatorVisitor(TImage* const in_image, Mask* const in_mask, TDescriptorMap& in_descriptorMap, const unsigned int in_half_width) :
  image(in_image), mask(in_mask), descriptorMap(in_descriptorMap),
  half_width(in_half_width)
  {
    
  }

  template <typename VertexType, typename Graph>
  void initialize_vertex(VertexType v, Graph& g) const
  { 
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index;
    index[0] = v[0];
    index[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, half_width);

    typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;

    DescriptorType descriptor(this->image, mask, region);
    put(descriptorMap, v, descriptor);
  };

  template <typename VertexType, typename Graph>
  void discover_vertex(VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void vertex_match_made(VertexType, VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  void paint_vertex(VertexType, VertexType, Graph&) const { };

  template <typename VertexType, typename Graph>
  bool accept_painted_vertex(VertexType, Graph&) const { return true; };

  template <typename VertexType, typename Graph>
  void finish_vertex(VertexType, Graph&) const { };
};

#endif
