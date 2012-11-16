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

#ifndef ReplayVisitor_HPP
#define ReplayVisitor_HPP

#include "Visitors/InpaintingVisitors/InpaintingVisitorParent.h"

// Concepts
#include "Concepts/DescriptorVisitorConcept.hpp"

// Priority
#include "Priority/Priority.h"

// Accept criteria
#include "ImageProcessing/BoundaryEnergy.h"

// Boost
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Helpers
#include "ITKHelpers/ITKHelpers.h"

/**
 * This is a visitor that replays an inpainting from a log file.
 */
template <typename TGraph, typename TImage, typename TBoundaryNodeQueue,
          typename TFillStatusMap, typename TBoundaryStatusMap>
struct ReplayVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TBoundaryNodeQueue& BoundaryNodeQueue;
  TFillStatusMap& FillStatusMap;
  TBoundaryStatusMap& BoundaryStatusMap;

  const unsigned int HalfWidth;

  ReplayVisitor(TImage* const in_image, Mask* const in_mask,
                TBoundaryNodeQueue& in_boundaryNodeQueue, TFillStatusMap& in_fillStatusMap,
                const unsigned int in_half_width, TBoundaryStatusMap& in_boundaryStatusMap) :
    Image(in_image), MaskImage(in_mask), BoundaryNodeQueue(in_boundaryNodeQueue),
    FillStatusMap(in_fillStatusMap), BoundaryStatusMap(in_boundaryStatusMap),
    HalfWidth(in_half_width)
  {
  }

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const override
  {
    itk::Index<2> target_index = ITKHelpers::CreateIndex(target);

    itk::Index<2> source_index = ITKHelpers::CreateIndex(source);

    assert(Image->GetLargestPossibleRegion().IsInside(source_index));
    assert(Image->GetLargestPossibleRegion().IsInside(target_index));

    Image->SetPixel(target_index, Image->GetPixel(source_index));
  }

  void FinishVertex(VertexDescriptorType v, VertexDescriptorType sourceNode) override
  {
    // Mark this pixel as filled, the area around it as filled, and the mask in this region as filled.
    // Determine the new boundary, and setup the nodes in the boundary queue.

    // Construct the region around the vertex
    itk::Index<2> indexToFinish = ITKHelpers::CreateIndex(v);

    itk::ImageRegion<2> regionToFinish = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, HalfWidth);

    regionToFinish.Crop(Image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Mark all the pixels in this region as filled. This must be done before creating
    // the mask image to use to check for boundary pixels.
    // It does not matter which image we iterate over, we just want the indices.
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(Image, regionToFinish);

    while(!gridIterator.IsAtEnd())
    {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(gridIterator.GetIndex());

      put(FillStatusMap, v, true);
      MaskImage->SetPixel(gridIterator.GetIndex(), MaskImage->GetValidValue());
      ++gridIterator;
    }

    // Initialize the newly filled vertices because they may now be valid source nodes.
    // You may not want to do this in some cases (i.e. if the descriptors needed cannot be
    // computed on newly filled regions)
    gridIterator.GoToBegin();
    while(!gridIterator.IsAtEnd())
    {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(gridIterator.GetIndex());

      InitializeVertex(v);
      ++gridIterator;
    }

    // Add pixels that are on the new boundary to the queue, and mark other pixels as not in the queue.
    itk::ImageRegionConstIteratorWithIndex<Mask> imageIterator(MaskImage, regionToFinish);

    while(!imageIterator.IsAtEnd())
    {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(imageIterator.GetIndex());

      // Mark all nodes in the patch around this node as filled (in the FillStatusMap).
      // This makes them ignored if they are still in the boundaryNodeQueue.
      if(ITKHelpers::HasNeighborWithValue(imageIterator.GetIndex(), MaskImage, MaskImage->GetHoleValue()))
      {
        put(BoundaryStatusMap, v, true);
        this->BoundaryNodeQueue.push(v);
      }
      else
      {
        put(BoundaryStatusMap, v, false);
      }

      ++imageIterator;
    }

  } // finish_vertex

  void InpaintingComplete() const override
  {
    ITKHelpers::WriteImage(Image, "output.mha");
  }

}; // Replay

#endif
