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

#ifndef ImagePatchInpaintingVisitor_HPP
#define ImagePatchInpaintingVisitor_HPP

#include "Priority/Priority.h"

#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

#include "InpaintingVisitorParent.h"

#include "Concepts/DescriptorConcept.hpp"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Submodules
#include <ITKHelpers/ITKHelpers.h>
#include <Helpers/Helpers.h>

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It creates
 * and differences ImagePatchPixelDescriptor objects at each pixel.
 */
template <typename TGraph, typename TImage, typename TBoundaryNodeQueue,
          typename TFillStatusMap, typename TDescriptorMap, typename TPriority,
          typename TPriorityMap, typename TBoundaryStatusMap>
struct ImagePatchInpaintingVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::property_traits<TDescriptorMap>::value_type DescriptorType;
  BOOST_CONCEPT_ASSERT((DescriptorConcept<DescriptorType, TGraph>));
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TBoundaryNodeQueue& BoundaryNodeQueue;
  TPriority* PriorityFunction;
  TFillStatusMap& FillStatusMap;
  TDescriptorMap& DescriptorMap;
  
  TPriorityMap& PriorityMap;
  TBoundaryStatusMap& BoundaryStatusMap;

  unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  ImagePatchInpaintingVisitor(TImage* const in_image, Mask* const in_mask,
                              TBoundaryNodeQueue& in_boundaryNodeQueue, TFillStatusMap& in_fillStatusMap,
                              TDescriptorMap& in_descriptorMap, TPriorityMap& in_priorityMap,
                              TPriority* const in_priorityFunction,
                              const unsigned int in_half_width, TBoundaryStatusMap& in_boundaryStatusMap) :
    Image(in_image), MaskImage(in_mask), BoundaryNodeQueue(in_boundaryNodeQueue), PriorityFunction(in_priorityFunction), FillStatusMap(in_fillStatusMap), DescriptorMap(in_descriptorMap),
    PriorityMap(in_priorityMap), BoundaryStatusMap(in_boundaryStatusMap), HalfWidth(in_half_width), NumberOfFinishedVertices(0)
  {
  }

  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  {
    //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
    // Create the patch object and associate with the node
    itk::Index<2> index;
    index[0] = v[0];
    index[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, HalfWidth);

    DescriptorType descriptor(this->image, this->mask, region);
    descriptor.SetVertex(v);
    put(DescriptorMap, v, descriptor);

  }

  void discover_vertex(VertexDescriptorType v, TGraph& g) const
  {
    itk::Index<2> index = {{v[0], v[1]}};
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->HalfWidth);

    // Create the list of valid pixels
    std::vector<itk::Index<2> > validPixels = this->MaskImage->GetValidPixelsInRegion(region);
    std::vector<itk::Offset<2> > validOffsets;
    for(size_t i = 0; i < validPixels.size(); ++i)
    {
      itk::Offset<2> offset = validPixels[i] - region.GetIndex();
      validOffsets.push_back(offset);
    }

    std::cout << "Discovered " << v[0] << " " << v[1] << std::endl;
    std::cout << "Priority: " << get(this->PriorityMap, v) << std::endl;
    DescriptorType& descriptor = get(this->DescriptorMap, v);
    descriptor.SetStatus(DescriptorType::TARGET_NODE);
    descriptor.SetValidOffsets(validOffsets);
  }

  void vertex_match_made(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    std::cout << "Match made: target: " << target[0] << " " << target[1]
              << " with source: " << source[0] << " " << source[1] << std::endl;
    assert(get(FillStatusMap, source));
    assert(get(DescriptorMap, source).IsFullyValid());
  }

  void paint_vertex(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    itk::Index<2> target_index;
    target_index[0] = target[0];
    target_index[1] = target[1];

    itk::Index<2> source_index;
    source_index[0] = source[0];
    source_index[1] = source[1];

    assert(image->GetLargestPossibleRegion().IsInside(source_index));
    assert(image->GetLargestPossibleRegion().IsInside(target_index));

    this->Image->SetPixel(target_index, this->Image->GetPixel(source_index));
  }

  bool accept_painted_vertex(VertexDescriptorType v, TGraph& g) const
  {
    return true;
  }

  void finish_vertex(VertexDescriptorType v, TGraph& g)
  {
    // Construct the region around the vertex
    itk::Index<2> indexToFinish;
    indexToFinish[0] = v[0];
    indexToFinish[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, HalfWidth);

    region.Crop(Image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Mark all the pixels in this region as filled. This must be done before creating
    // the mask image to use to check for boundary pixels.
    // It does not matter which image we iterate over, we just want the indices.
    // Additionally, initialize these vertices because they may now be valid.
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(Image, region);

    while(!gridIterator.IsAtEnd())
    {
      VertexDescriptorType v;
      v[0] = gridIterator.GetIndex()[0];
      v[1] = gridIterator.GetIndex()[1];
      put(FillStatusMap, v, true);
      MaskImage->SetPixel(gridIterator.GetIndex(), MaskImage->GetValidValue());
      ++gridIterator;
    }

    gridIterator.GoToBegin();
    while(!gridIterator.IsAtEnd())
    {
      VertexDescriptorType v;
      v[0] = gridIterator.GetIndex()[0];
      v[1] = gridIterator.GetIndex()[1];
      initialize_vertex(v, g);
      ++gridIterator;
    }

    // Update the priority function.
    this->priorityFunction->Update(indexToFinish);

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(MaskImage, region);

    while(!imageIterator.IsAtEnd())
    {
      VertexDescriptorType v;
      v[0] = imageIterator.GetIndex()[0];
      v[1] = imageIterator.GetIndex()[1];

      // Mark all nodes in the patch around this node as filled (in the FillStatusMap).
      // This makes them ignored if they are still in the boundaryNodeQueue.
      if(ITKHelpers::HasNeighborWithValue(imageIterator.GetIndex(), MaskImage, MaskImage->GetHoleValue()))
      {
        put(BoundaryStatusMap, v, true);

        // Note: the priority must be set before the node is pushed into the queue.
        float priority = this->priorityFunction->ComputePriority(imageIterator.GetIndex());
        //std::cout << "updated priority: " << priority << std::endl;
        put(PriorityMap, v, priority);
        this->boundaryNodeQueue.push(v);
      }
      else
      {
        put(BoundaryStatusMap, v, false);
      }

      ++imageIterator;
    }

    {
      // Debug only - write the mask to a file
      ITKHelpers::WriteImage(this->MaskImage, Helpers::GetSequentialFileName("debugMask", this->NumberOfFinishedVertices, "png"));
      ITKHelpers::WriteVectorImageAsRGB(this->Image, Helpers::GetSequentialFileName("output", this->NumberOfFinishedVertices, "png"));
      this->NumberOfFinishedVertices++;
    }
  } // finish_vertex

}; // ImagePatchInpaintingVisitor

#endif
