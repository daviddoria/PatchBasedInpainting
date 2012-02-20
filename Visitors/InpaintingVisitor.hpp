#ifndef InpaintingVisitor_HPP
#define InpaintingVisitor_HPP

#include "Visitors/InpaintingVisitorParent.h"

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
#include "Helpers/ITKHelpers.h"
#include "Helpers/OutputHelpers.h"

/**
 * This is a visitor that complies with the InpaintingVisitorConcept. It forwards
 * initialize_vertex and discover_vertex, the only two functions that need to know about
 * the descriptor type, to a visitor that models DescriptorVisitorConcept.
 * The visitor needs to know the patch size of the patch to be inpainted because it uses
 * this size to traverse the inpainted region to update the boundary.
 */
template <typename TGraph, typename TImage, typename TBoundaryNodeQueue,
          typename TFillStatusMap, typename TDescriptorVisitor, typename TPriority,
          typename TPriorityMap, typename TBoundaryStatusMap>
struct InpaintingVisitor : public InpaintingVisitorParent<TGraph>
{
  BOOST_CONCEPT_ASSERT((DescriptorVisitorConcept<TDescriptorVisitor, TGraph>));

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TImage* Image;
  Mask* MaskImage;
  TBoundaryNodeQueue& BoundaryNodeQueue;
  TPriority* PriorityFunction;
  TFillStatusMap& FillStatusMap;
  TDescriptorVisitor& DescriptorVisitor;

  TPriorityMap& PriorityMap;
  TBoundaryStatusMap& BoundaryStatusMap;

  const unsigned int HalfWidth;

  InpaintingVisitor(TImage* const in_image, Mask* const in_mask,
                    TBoundaryNodeQueue& in_boundaryNodeQueue, TFillStatusMap& in_fillStatusMap,
                    TDescriptorVisitor& in_descriptorVisitor, TPriorityMap& in_priorityMap,
                    TPriority* const in_priorityFunction,
                    const unsigned int in_half_width, TBoundaryStatusMap& in_boundaryStatusMap) :
  Image(in_image), MaskImage(in_mask), BoundaryNodeQueue(in_boundaryNodeQueue), PriorityFunction(in_priorityFunction),
  FillStatusMap(in_fillStatusMap), DescriptorVisitor(in_descriptorVisitor),
  PriorityMap(in_priorityMap), BoundaryStatusMap(in_boundaryStatusMap),
  HalfWidth(in_half_width)
  {
  }

  void InitializeVertex(VertexDescriptorType v) const
  {
    DescriptorVisitor.initialize_vertex(v);
  };

  void DiscoverVertex(VertexDescriptorType v) const
  {
    DescriptorVisitor.discover_vertex(v);
  };

  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source)
  {
    assert(get(FillStatusMap, source));
  };

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const
  {
    itk::Index<2> target_index = ITKHelpers::CreateIndex(target);

    itk::Index<2> source_index = ITKHelpers::CreateIndex(source);

    assert(Image->GetLargestPossibleRegion().IsInside(source_index));
    assert(Image->GetLargestPossibleRegion().IsInside(target_index));

    Image->SetPixel(target_index, Image->GetPixel(source_index));
  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    // return true;
    BoundaryEnergy<TImage> boundaryEnergy(Image, MaskImage);

    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    float energy = boundaryEnergy(sourceRegion, targetRegion);
    std::cout << "Energy: " << energy << std::endl;

    float energyThreshold = 100;
    if(energy < energyThreshold)
      {
      std::cout << "Match accepted." << std::endl;
      return true;
      }
    else
      {
      std::cout << "Match rejected." << std::endl;
      return false;
      }
  };

  void FinishVertex(VertexDescriptorType targetNode, VertexDescriptorType sourceNode)
  {
    // Mark this pixel and the area around it as filled, and mark the mask in this region as filled.
    // Determine the new boundary, and setup the nodes in the boundary queue.

    // Construct the region around the vertex
    itk::Index<2> indexToFinish = ITKHelpers::CreateIndex(targetNode);

    itk::ImageRegion<2> regionToFinish = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, HalfWidth);

    regionToFinish.Crop(Image->GetLargestPossibleRegion()); // Make sure the region is entirely inside the image

    // Mark all the pixels in this region as filled.
    {
    itk::ImageRegionConstIteratorWithIndex<Mask> maskIterator(MaskImage, regionToFinish);

    while(!maskIterator.IsAtEnd())
      {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(maskIterator.GetIndex());

      put(FillStatusMap, v, true);
      MaskImage->MarkAsValid(maskIterator.GetIndex());
      ++maskIterator;
      }
    }

    // Initialize the newly filled vertices because they may now be valid source nodes.
    // You may not want to do this in some cases (i.e. if the descriptors needed cannot be 
    // computed on newly filled regions)
    {
    itk::ImageRegionConstIteratorWithIndex<TImage> gridIterator(Image, regionToFinish);
    while(!gridIterator.IsAtEnd())
      {
      VertexDescriptorType v = Helpers::ConvertFrom<VertexDescriptorType, itk::Index<2> >(gridIterator.GetIndex());

      InitializeVertex(v);
      ++gridIterator;
      }
    }

    // Update the priority function.
    this->PriorityFunction->Update(sourceNode, targetNode);

    {
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
        float priority = this->PriorityFunction->ComputePriority(imageIterator.GetIndex());
        //std::cout << "updated priority: " << priority << std::endl;
        put(PriorityMap, v, priority);
        }
      else
        {
        put(BoundaryStatusMap, v, false);
        }

      ++imageIterator;
      }
    }

  }; // finish_vertex

  void InpaintingComplete() const
  {
    OutputHelpers::WriteImage(Image, "output.mha");
  }

}; // InpaintingVisitor

#endif
